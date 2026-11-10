// SlitScan -- does the vertical or horizontal slit scaning from left and right
//             image sources. Will do an output frame as a signal.
//
// This is the gel sink. It knows nothing about where its frames come
// from: gel sources push frames in for the left and the right side,
// and on every tick the sink cuts slits from the latest frame of
// each side, integrates them, and broadcasts the finished picture.
//
// The algorithm is the one worked out in playground/SSTest.cpp:
//
//   + each side has a "strip", a flat image half the output wide.
//     A new slit goes in at x = 0 and everything already there
//     shifts along by one slit pitch, so the strip is a record of
//     the slits over time, newest first.
//   + the strip is then put through a perspective transform that
//     leaves the newest edge at full height and magnifies the
//     oldest edge by the perspective factor.
//   + the right half of the picture is that transform as it is, and
//     the left half is its mirror image, so the newest slits meet
//     in the middle and stream outwards.
//
// What differs from SSTest:
//
//   + it is driven by time, not by a count of frames. Slits are
//     produced at slits_per_second no matter the tick rate.
//   + the slit can sweep across the frame. A still image needs
//     that, or every slit would be the same.
//
// Threading: everything public is to be called from the GUI thread,
// and never blocks. The work is done in one worker thread, which is
// fed through a GelQueue and owns all of the scan state. Finished
// frames come back to the GUI thread by way of a Glib::Dispatcher.
#pragma once

#include <algorithm>
#include <cmath>

namespace Gel {

  class SlitScan : public sigc::trackable {
  public:
    enum class Side { Left, Right };

    struct Params {
      int width  = 1280;                // size of the output frames
      int height = 800;

      int slit_width   = 3;             // pixels, in the frame and in the strip
      int slit_spacing = 0;             // blank pixels between slits
      double perspective_factor = 10.0; // magnification at the outer edge

      double slits_per_second = 30.0;   // slits cut per second of time

      // how fast the slit sweeps across the frame, in frame widths
      // per second. It goes over and back. 0 holds the slit still in
      // the centre, which is what you want for cameras and videos.
      double sweep_rate = 0.05;

      // with gels on one side only, show them on both sides.
      bool mirror_missing_side = true;

      bool operator==(const Params&) const = default;
    };

    SlitScan() : jobs(64) {
      m_dispatcher.connect(sigc::mem_fun(*this, &SlitScan::on_frame_ready));
      tick_server = IMasterTickServer::create();
      tick_server->signal_tick().connect(sigc::mem_fun(*this, &SlitScan::on_tick));
      worker_thread = std::thread(&SlitScan::worker, this);
    }

    ~SlitScan() {
      jobs.shutdown();
      if (worker_thread.joinable())
        worker_thread.join();
    }

    SlitScan(const SlitScan&) = delete;
    SlitScan& operator=(const SlitScan&) = delete;

    // Sets the parameters. Cheap to call again and again: it does
    // nothing unless something changed. A change of size, slit or
    // spacing starts the strips afresh.
    void configure(const Params& params_) {
      if (configured && params_ == requested)
        return;
      Job job;
      job.kind = Job::Kind::Configure;
      job.params = params_;
      if (jobs.try_enqueue(std::move(job))) {
        requested = params_;
        configured = true;
      }
    }

    // A new frame for one side. The frame must be 8-bit RGB and
    // must not be modified afterwards. Hook these up to
    // GelSource::signal_next_frame().
    void push_frame(Side side, const cv::Mat& frame, time_stamp ts) {
      if (frame.empty() || frame.type() != CV_8UC3)
        return;
      Job job;
      job.kind = Job::Kind::Frame;
      job.side = side;
      job.frame = frame;
      job.ts = ts;
      jobs.try_enqueue(std::move(job)); // if we are swamped, drop it
    }

    // Call when playback (re)starts, so that the time spent paused
    // is not taken for time to be scanned.
    void resume() {
      Job job;
      job.kind = Job::Kind::Resume;
      jobs.try_enqueue(std::move(job));
    }

    // Wipes the strips and forgets the frames.
    void clear() {
      Job job;
      job.kind = Job::Kind::Clear;
      jobs.try_enqueue(std::move(job));
    }

    // The finished frames, RGB, emitted in the GUI thread.
    sig_next_frame& signal_next_frame() { return m_sig_next_frame; }

  protected:
    // GUI thread. Ask the worker for the next output frame.
    void on_tick(time_stamp tick) {
      Job job;
      job.kind = Job::Kind::Tick;
      job.ts = tick;
      jobs.try_enqueue(std::move(job));
    }

    // GUI thread, by way of the dispatcher.
    void on_frame_ready() {
      cv::Mat frame;
      time_stamp ts;
      {
        lockguard lock(out_mutex);
        frame = out_frame;
        ts = out_ts;
      }
      if (!frame.empty())
        m_sig_next_frame.emit(frame, ts);
    }

  private:
    struct Job {
      enum class Kind { Configure, Frame, Tick, Resume, Clear };
      Kind kind = Kind::Tick;
      Side side = Side::Left;
      cv::Mat frame;
      time_stamp ts;
      Params params;
    };

    // the scan state of one side. Worker thread only.
    struct SideState {
      cv::Mat strip;      // the integrated slits, newest at x = 0
      cv::Mat scratch;    // same size; we shift into it and swap
      cv::Mat frame;      // latest frame from the source
      double phase = 0.0; // where the slit is in its sweep
    };

    // ---- GUI thread state
    unique_ptr<IMasterTickServer> tick_server;
    Glib::Dispatcher m_dispatcher;
    sig_next_frame m_sig_next_frame;
    Params requested;
    bool configured = false;

    // ---- shared between the two threads
    GelQueue<Job> jobs;
    std::mutex out_mutex; // guards out_frame and out_ts
    cv::Mat out_frame;
    time_stamp out_ts;

    // ---- worker thread state
    std::thread worker_thread;
    Params p;
    bool ready = false;         // have we been configured?
    SideState left;
    SideState right;
    cv::Mat perspective;        // strip -> one half of the picture
    bool have_last_tick = false;
    time_stamp last_tick;
    double slit_debt = 0.0;     // fractional slits carried to the next tick

    static Params sanitized(Params q) {
      q.width  = std::max(q.width, 2);
      q.height = std::max(q.height, 1);
      q.slit_width   = std::max(q.slit_width, 1);
      q.slit_spacing = std::max(q.slit_spacing, 0);
      q.perspective_factor = std::max(q.perspective_factor, 1.0);
      q.slits_per_second   = std::clamp(q.slits_per_second, 0.0, 10000.0);
      return q;
    }

    void worker() {
      while (auto job = jobs.dequeue()) {
        switch (job->kind) {
        case Job::Kind::Configure:
          apply_params(sanitized(job->params));
          break;

        case Job::Kind::Frame:
          side_state(job->side).frame = job->frame;
          break;

        case Job::Kind::Tick:
          if (ready) {
            integrate(job->ts);
            compose(job->ts);
          }
          break;

        case Job::Kind::Resume:
          have_last_tick = false;
          break;

        case Job::Kind::Clear:
          for (SideState* s : {&left, &right}) {
            s->frame.release();
            s->phase = 0.0;
            if (!s->strip.empty()) s->strip.setTo(cv::Scalar::all(0));
          }
          slit_debt = 0.0;
          have_last_tick = false;
          break;
        }
      }
    }

    SideState& side_state(Side side) {
      return side == Side::Left ? left : right;
    }

    void apply_params(const Params& q) {
      const bool geometry_changed = !ready
        || q.width  != p.width
        || q.height != p.height
        || q.slit_width   != p.slit_width
        || q.slit_spacing != p.slit_spacing;
      p = q;

      const int half = p.width / 2;
      const double w = half;
      const double h = p.height;

      if (geometry_changed) {
        for (SideState* s : {&left, &right}) {
          s->strip   = cv::Mat::zeros(p.height, half, CV_8UC3);
          s->scratch = cv::Mat::zeros(p.height, half, CV_8UC3);
        }
        slit_debt = 0.0;
      }

      // Straight from SSTest: the corners of the strip, and where
      // we want them to go. The edge at x = 0 (the newest slits)
      // keeps its height; the far edge is stretched up and down
      // so that it is perspective_factor times as high.
      const float higher = float(h * (p.perspective_factor - 1.0) / 2.0);

      const cv::Point2f src_points[4] = {
        cv::Point2f(0, 0)
        , cv::Point2f(float(w - 1), 0)
        , cv::Point2f(float(w - 1), float(h - 1))
        , cv::Point2f(0, float(h - 1))
      };
      const cv::Point2f dst_points[4] = {
        cv::Point2f(0, 0)
        , cv::Point2f(float(w), -higher)
        , cv::Point2f(float(w), float(h) + higher)
        , cv::Point2f(0, float(h))
      };
      perspective = cv::getPerspectiveTransform(src_points, dst_points);
      ready = true;
    }

    // Where the left edge of the slit is, for a sweep phase. The
    // slit runs across the frame and back, one way per unit of phase.
    int slit_x(int range, double phase) const {
      if (range <= 0)
        return 0;
      if (p.sweep_rate == 0.0)
        return range / 2;

      double t = std::fmod(phase, 2.0);
      if (t < 0.0) t += 2.0;
      const double u = (t <= 1.0) ? t : 2.0 - t;
      return std::clamp(int(std::lround(u * range)), 0, range);
    }

    // Cut the slits that are due and add them to the strips.
    void integrate(time_stamp tick) {
      int slits = 0;
      if (have_last_tick) {
        // never try to make up for more than a quarter second.
        const double dt = std::clamp(seconds_between(last_tick, tick), 0.0, 0.25);
        slit_debt += dt * p.slits_per_second;
        slits = int(slit_debt);
        slit_debt -= slits;
      }
      last_tick = tick;
      have_last_tick = true;

      for (SideState* s : {&left, &right})
        add_slits(*s, slits);
    }

    void add_slits(SideState& s, int slits) {
      if (slits <= 0 || s.frame.empty())
        return;

      const int sw    = p.slit_width;
      const int pitch = sw + p.slit_spacing;
      const int wide  = s.strip.cols;
      const int high  = s.strip.rows;
      const int shift = (slits >= wide) ? wide : std::min(slits * pitch, wide);

      // move what is there along by `shift`, into the scratch
      // image, blank the gap at the front, and swap.
      if (shift < wide) {
        s.strip(cv::Rect(0, 0, wide - shift, high))
          .copyTo(s.scratch(cv::Rect(shift, 0, wide - shift, high)));
      }
      s.scratch(cv::Rect(0, 0, shift, high)).setTo(cv::Scalar::all(0));
      std::swap(s.strip, s.scratch);

      // now the new slits. Slit 0 is the newest and sits at x = 0;
      // the others were cut earlier in the sweep and sit behind it.
      const double step = (p.slits_per_second > 0.0)
        ? p.sweep_rate / p.slits_per_second
        : 0.0;
      s.phase += slits * step;

      const int src_w = std::min(sw, s.frame.cols);
      cv::Mat sized;
      cv::Mat mirrored;

      for (int j = 0; j < slits; ++j) {
        const int x_dst = j * pitch;
        if (x_dst >= wide)
          break;
        const int w_dst = std::min(sw, wide - x_dst);

        const int x_src = slit_x(s.frame.cols - src_w, s.phase - j * step);
        const cv::Mat slit = s.frame(cv::Rect(x_src, 0, src_w, s.frame.rows));

        if (slit.cols != sw || slit.rows != high)
          cv::resize(slit, sized, cv::Size(sw, high), 0, 0, cv::INTER_LINEAR);
        else
          sized = slit;

        // as in SSTest, the slit goes in mirrored, so that the
        // pixels inside a slit run the same way as the slits do.
        cv::flip(sized, mirrored, 1);
        mirrored(cv::Rect(0, 0, w_dst, high))
          .copyTo(s.strip(cv::Rect(x_dst, 0, w_dst, high)));
      }
    }

    // Build the output frame from the two strips and hand it over.
    void compose(time_stamp tick) {
      const int half = p.width / 2;
      const cv::Size half_size(half, p.height);

      const SideState* r = &right;
      const SideState* l = &left;
      if (p.mirror_missing_side) {
        if (right.frame.empty() && !left.frame.empty()) r = &left;
        if (left.frame.empty() && !right.frame.empty()) l = &right;
      }

      // always a fresh Mat; the GUI may still be painting the last one.
      cv::Mat out = cv::Mat::zeros(p.height, p.width, CV_8UC3);
      cv::Mat warped;

      if (!r->frame.empty()) {
        cv::warpPerspective(r->strip, warped, perspective, half_size);
        warped.copyTo(out(cv::Rect(p.width - half, 0, half, p.height)));
      }

      if (!l->frame.empty()) {
        if (l != r || warped.empty())
          cv::warpPerspective(l->strip, warped, perspective, half_size);
        cv::Mat mirrored;
        cv::flip(warped, mirrored, 1);
        mirrored.copyTo(out(cv::Rect(0, 0, half, p.height)));
      }

      {
        lockguard lock(out_mutex);
        out_frame = out;
        out_ts = tick;
      }
      m_dispatcher.emit();
    }
  };
}
