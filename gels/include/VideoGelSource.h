// VideoGelSource.h -- GelSource for videos
#pragma once

namespace Gel {
  using namespace std;

  // What videos and cameras have in common: frames come out of a
  // cv::VideoCapture. Reading one can take a while -- a camera
  // makes you wait for its next frame -- so a reader thread does
  // the reading and keeps the latest frame, and on every tick the
  // GUI thread broadcasts whatever is the latest.
  //
  // The reader thread only runs while the source is active, or
  // while the configuration dialog is watching.
  class CaptureGelSource : public GelSource {
  public:
    ~CaptureGelSource() override { stop_reader(); }

    bool is_moving() const override { return true; }

    // true once the capture could not be opened or read
    bool has_failed() const { return failed; }

    // the latest frame as it came, for the configuration dialog
    cv::Mat preview_frame() override {
      lockguard lock(frame_mutex);
      return latest;
    }

    void set_previewing(bool on) override {
      previewing = on;
      update_reader();
    }

    // A video goes back to its first frame. The reader thread does
    // the rewinding, the next time round or when it next runs; in
    // the meantime the frame we have is from before, so it goes.
    void restart() override {
      GelSource::restart();
      rewind_requested = true;
      lockguard lock(frame_mutex);
      latest.release();
    }

    // BGR, grey or BGRA, as OpenCV hands frames out, to our RGB.
    // Always gives a fresh Mat.
    static cv::Mat to_rgb(const cv::Mat& frame) {
      cv::Mat rgb;
      switch (frame.channels()) {
      case 1:  cv::cvtColor(frame, rgb, cv::COLOR_GRAY2RGB); break;
      case 4:  cv::cvtColor(frame, rgb, cv::COLOR_BGRA2RGB); break;
      default: cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);  break;
      }
      if (rgb.depth() != CV_8U)
        rgb.convertTo(rgb, CV_8U);
      return rgb;
    }

  protected:
    using Opener = std::function<bool(cv::VideoCapture&)>;

    // `opener_` opens the capture, in the reader thread. `is_file_`
    // says whether this is a file, to be played at its own frame
    // rate and looped, or a live device, to be read as fast as it
    // delivers.
    CaptureGelSource(Opener opener_, bool is_file_, string name_)
      : opener(std::move(opener_))
      , is_file(is_file_)
      , name(std::move(name_)) {
      signal_tick().connect(sigc::mem_fun(*this, &CaptureGelSource::on_tick));
    }

    void on_activated() override { update_reader(); }
    void on_deactivated() override { update_reader(); }

    // GUI thread
    void on_tick(time_stamp tick) {
      if (!is_active())
        return;

      // the reader came to the end of the video and went round again
      const int ended_now = times_ended.load();
      if (ended_now != times_ended_seen) {
        times_ended_seen = ended_now;
        announce_finished();
        if (!is_active())
          return; // whoever heard that has moved on to another gel
      }

      cv::Mat frame;
      {
        lockguard lock(frame_mutex);
        frame = latest;
      }
      if (frame.empty())
        return;

      broadcast_next_frame(transformed(frame, tick), tick);
    }

  private:
    // the reader runs for the scan, for the dialog, or for both
    void update_reader() {
      if (is_active() || previewing)
        start_reader();
      else
        stop_reader();
    }

    void start_reader() {
      if (running.exchange(true))
        return;
      if (reader_thread.joinable())
        reader_thread.join();
      failed = false;
      reader_thread = std::thread(&CaptureGelSource::reader, this);
    }

    void stop_reader() {
      running = false;
      if (reader_thread.joinable())
        reader_thread.join();
    }

    // The reader thread. It alone touches `capture` while it runs;
    // the capture stays open between runs, so a paused video
    // carries on from where it was.
    void reader() {
      if (!capture.isOpened() && !(opener(capture) && capture.isOpened())) {
        cerr << "SlitScan: cannot open " << name << endl;
        failed = true;
        return;
      }

      double fps = is_file ? capture.get(cv::CAP_PROP_FPS) : 0.0;
      if (!(fps >= 1.0 && fps <= 1000.0))
        fps = 30.0;
      const auto period = std::chrono::duration_cast<time_basis::duration>
        (std::chrono::duration<double>(1.0 / fps));

      auto next_frame = time_basis::now();
      int misses = 0;
      cv::Mat raw;

      while (running) {
        if (is_file && rewind_requested.exchange(false)) {
          capture.set(cv::CAP_PROP_POS_FRAMES, 0);
          next_frame = time_basis::now();
        }

        if (!capture.read(raw) || raw.empty()) {
          if (is_file && misses == 0) {
            // the end of the video: go round again
            capture.set(cv::CAP_PROP_POS_FRAMES, 0);
            ++misses;
            ++times_ended;
            continue;
          }
          if (++misses > 200) {
            cerr << "SlitScan: no frames from " << name << endl;
            failed = true;
            break;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
          continue;
        }
        misses = 0;

        cv::Mat rgb = to_rgb(raw);
        {
          lockguard lock(frame_mutex);
          latest = rgb;
        }

        if (is_file) {
          // play at the video's own frame rate
          next_frame += period;
          const auto now = time_basis::now();
          if (next_frame < now)
            next_frame = now; // we are behind; do not try to catch up
          else
            std::this_thread::sleep_until(next_frame);
        }
      }

      // let go of a live device, so that the camera is not kept
      // on while we are not looking. A video file stays open.
      if (!is_file)
        capture.release();
    }

    Opener opener;
    bool is_file;
    string name;

    cv::VideoCapture capture;
    std::thread reader_thread;
    std::atomic<bool> running{false};
    std::atomic<bool> failed{false};
    std::atomic<int> times_ended{0}; // how often a video has reached its end
    std::atomic<bool> rewind_requested{false}; // see restart()

    std::mutex frame_mutex; // guards latest
    cv::Mat latest;         // RGB

    // GUI thread only
    bool previewing = false; // the configuration dialog is watching
    int times_ended_seen = 0;
  };

  // A video file as a gel. It plays at its own frame rate, whatever
  // the tick rate is, and loops.
  class VideoGelSource : public CaptureGelSource {
  public:
    explicit VideoGelSource(const string& pathname)
      : CaptureGelSource([pathname](cv::VideoCapture& cap) { return cap.open(pathname); }
                         , true
                         , pathname) {}

    // The first frame of a video, as RGB, for thumbnails and
    // previews. Empty if the file is not a video we can read.
    static cv::Mat first_frame(const string& pathname) {
      cv::VideoCapture cap;
      cv::Mat raw;
      if (!cap.open(pathname) || !cap.read(raw) || raw.empty())
        return {};
      return to_rgb(raw);
    }
  };
}
