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
  // The reader thread only runs while the source is active.
  class CaptureGelSource : public GelSource {
  public:
    ~CaptureGelSource() override { stop_reader(); }

    bool is_moving() const override { return true; }

    // true once the capture could not be opened or read
    bool has_failed() const { return failed; }

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

    void on_activated() override {
      started = false;
      start_reader();
    }

    void on_deactivated() override { stop_reader(); }

    // GUI thread
    void on_tick(time_stamp tick) {
      if (!is_active())
        return;

      cv::Mat frame;
      {
        lockguard lock(frame_mutex);
        frame = latest;
      }
      if (frame.empty())
        return;

      if (!started) {
        start_time = tick;
        started = true;
      }
      broadcast_next_frame(transformed(frame, seconds_between(start_time, tick))
                           , tick);
    }

  private:
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
        if (!capture.read(raw) || raw.empty()) {
          if (is_file && misses == 0) {
            // the end of the video: go round again
            capture.set(cv::CAP_PROP_POS_FRAMES, 0);
            ++misses;
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

    std::mutex frame_mutex; // guards latest
    cv::Mat latest;         // RGB

    // GUI thread only
    bool started = false;
    time_stamp start_time;
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
