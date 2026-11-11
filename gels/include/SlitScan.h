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
//   + it is filtered against aliasing. The new slits of a tick are
//     one seamless stretch of the frame rather than pieces cut at
//     rounded positions, and the perspective is rendered by
//     render_half() rather than by warpPerspective.
//
// The horizontal slit scan is the same thing lying on its side, and
// that is just how it is made: the frames are transposed (rows for
// columns) on the way in, everything in between works exactly as
// for the vertical scan on a picture with its width and height
// exchanged, and the finished picture is transposed back on the way
// out. The left side of the scan comes out as the top.
//
// It can also record: the finished frames are handed to a Recorder,
// which writes them to a video file in a thread of its own.
//
// Threading: everything public is to be called from the GUI thread,
// and never blocks. The work is done in one worker thread, which is
// fed through a GelQueue and owns all of the scan state. Finished
// frames come back to the GUI thread by way of a Glib::Dispatcher.
#pragma once

#include <ScanEffect.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace Gel {

  class SlitScan : public sigc::trackable {
  public:
    enum class Side { Left, Right };

    struct Params {
      int width  = 1280;                // size of the output frames
      int height = 800;

      ScanEffect effect = ScanEffect::VerticalSlit; // which way the slit lies

      int slit_width   = 3;             // pixels, in the frame and in the strip
      int slit_spacing = 0;             // blank pixels between slits
      double perspective_factor = 10.0; // magnification at the outer edge

      double slits_per_second = 30.0;   // slits cut per second of time

      // how fast the slit sweeps across the frame, in frame widths
      // per second. It goes over and back. 0 holds the slit still
      // in the centre.
      double sweep_rate = 0.05;

      // does the slit sweep at all, on each side? A still image
      // needs it. For a video or a camera the picture moves by
      // itself, and the slit is better held still in the centre.
      bool sweep_left  = true;
      bool sweep_right = true;

      // is a side being fed several gels, one after the other? Then
      // the slit crosses each gel just once, instead of going over
      // and back, and at the far edge it says so by way of
      // signal_pass_finished() and waits there for the next gel.
      bool sequence_left  = false;
      bool sequence_right = false;

      // with gels on one side only, show them on both sides.
      bool mirror_missing_side = true;

      bool operator==(const Params&) const = default;
    };

    SlitScan() : jobs(64) {
      m_dispatcher.connect(sigc::mem_fun(*this, &SlitScan::on_frame_ready));
      m_pass_dispatcher.connect(sigc::mem_fun(*this, &SlitScan::on_pass_finished));
      tick_server = IMasterTickServer::create();
      tick_server->signal_tick().connect(sigc::mem_fun(*this, &SlitScan::on_tick));
      worker_thread = std::thread(&SlitScan::worker, this);
    }

    ~SlitScan() {
      // first, so that a worker waiting on a slow encoder is let go
      recorder.stop();
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
    //
    // `gel` says which gel the frame is from. It can be any number,
    // as long as it changes when the gel does: a frame from another
    // gel starts the slit off again from the near edge.
    void push_frame(Side side, const cv::Mat& frame, time_stamp ts, unsigned gel = 0) {
      if (frame.empty() || frame.type() != CV_8UC3)
        return;
      Job job;
      job.kind = Job::Kind::Frame;
      job.side = side;
      job.frame = frame;
      job.ts = ts;
      job.gel = gel;
      jobs.try_enqueue(std::move(job)); // if we are swamped, drop it
    }

    // Call when playback stops. Ticks that are still waiting their
    // turn are forgotten, so that the picture stops when the user
    // says stop, and does not first work through a backlog. Never
    // blocks.
    void discard_pending() { ++epoch; }

    // Call when playback (re)starts, so that the time spent paused
    // is not taken for time to be scanned.
    void resume() {
      Job job;
      job.kind = Job::Kind::Resume;
      jobs.try_enqueue(std::move(job));
    }

    // Starts writing the finished frames to a video file, one per
    // tick. Returns false, and records nothing, if the file cannot
    // be written or no encoder can be had for it. See Recorder.h.
    bool start_recording(const Recorder::Request& request) {
      return recorder.start(request);
    }

    // No more frames for the video file. Returns at once, and never
    // waits for the encoder: the file is closed in the background,
    // and signal_recording_finished() tells when that is done.
    // Returns false if nothing was being recorded.
    bool stop_recording() { return recorder.stop(); }

    bool is_recording() { return recorder.is_recording(); }

    // the codec and encoder the recording is being written with
    std::string recording_description() { return recorder.description(); }

    // Emitted in the GUI thread when a recording's file has been
    // closed, with how many frames went into it.
    sigc::signal<void(const Recorder::Result&)>& signal_recording_finished() {
      return recorder.signal_finished();
    }

    // Wipes the strips and forgets the frames.
    //
    // The request is a flag the worker looks at before every job,
    // so it cannot be lost however busy the worker is; the job
    // only serves to wake a worker that has nothing else to do.
    void clear() {
      clear_requested = true;
      {
        lockguard lock(out_mutex);
        out_frame.release(); // a frame on its way to the GUI is void
      }
      Job job;
      job.kind = Job::Kind::Clear;
      jobs.try_enqueue(std::move(job));
    }

    // The finished frames, RGB, emitted in the GUI thread.
    sig_next_frame& signal_next_frame() { return m_sig_next_frame; }

    // Emitted in the GUI thread when the slit has crossed the whole
    // of a gel on a side that has several (see Params). This is
    // the moment to send that side its next gel.
    sigc::signal<void(Side)>& signal_pass_finished() { return m_sig_pass_finished; }

  protected:
    // GUI thread. Ask the worker for the next output frame.
    void on_tick(time_stamp tick) {
      Job job;
      job.kind = Job::Kind::Tick;
      job.ts = tick;
      job.epoch = epoch.load();
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

    // GUI thread, by way of the dispatcher.
    void on_pass_finished() {
      if (pass_finished_left.exchange(false))
        m_sig_pass_finished.emit(Side::Left);
      if (pass_finished_right.exchange(false))
        m_sig_pass_finished.emit(Side::Right);
    }

  private:
    struct Job {
      enum class Kind { Configure, Frame, Tick, Resume, Clear };
      Kind kind = Kind::Tick;
      Side side = Side::Left;
      cv::Mat frame;
      time_stamp ts;
      Params params;
      unsigned epoch = 0; // of a Tick: see discard_pending()
      unsigned gel = 0;   // of a Frame: which gel it is from
    };

    // the scan state of one side. Worker thread only.
    struct SideState {
      cv::Mat strip;      // the integrated slits, newest at x = 0
      cv::Mat scratch;    // same size; we shift into it and swap
      cv::Mat frame;      // latest frame from the source
      double phase = 0.0; // where the slit is in its sweep
      unsigned gel = 0;   // which gel the frame is from
      bool pass_reported = false; // told the GUI this gel has been crossed

      // the frame scaled to the height of the strip, so that cutting
      // slits is a matter of columns only. Made when it is needed.
      cv::Mat prepared;
      bool prepared_stale = true;

      // where the slit was, and what it saw, at the last tick. The
      // new slits fill in smoothly between then and now.
      bool have_prev = false;
      double prev_u = 0.5;  // slit position, 0..1 across the frame
      cv::Mat prev_column;  // one column, the height of the strip
    };

    // ---- GUI thread state
    unique_ptr<IMasterTickServer> tick_server;
    Glib::Dispatcher m_dispatcher;
    sig_next_frame m_sig_next_frame;
    Glib::Dispatcher m_pass_dispatcher;
    sigc::signal<void(Side)> m_sig_pass_finished;
    Params requested;
    bool configured = false;

    // ---- shared between the two threads
    GelQueue<Job> jobs;
    std::atomic<unsigned> epoch{0}; // goes up each time playback stops
    std::atomic<bool> clear_requested{false};      // see clear()
    std::atomic<bool> pass_finished_left{false};  // set by the worker,
    std::atomic<bool> pass_finished_right{false}; // taken by the GUI
    std::mutex out_mutex; // guards out_frame and out_ts
    cv::Mat out_frame;
    time_stamp out_ts;

    // The recording. It has a thread of its own; see Recorder.h.
    Recorder recorder;

    // ---- worker thread state
    std::thread worker_thread;
    Params p;
    bool ready = false;         // have we been configured?
    SideState left;
    SideState right;
    cv::Mat perspective;        // strip -> one half of the picture

    // The perspective transform, taken apart for rendering. It
    // keeps vertical lines vertical, so it comes down to two simple
    // things: every column of the picture is a squeezed span of
    // columns of the strip, stretched up and down about the middle.
    // Knowing the span is what lets us average over all of it and
    // keep the picture free of jaggies; see render_half(). These
    // are the tables for it, each the size of a half picture.
    cv::Mat span_lo_x;   // per column: left edge of its span in the strip
    cv::Mat span_hi_x;   //             right edge
    cv::Mat span_y;      // per pixel: the row of the strip it comes from
    cv::Mat span_weight; // 1 / width of the span, to turn a sum into an average

    bool have_last_tick = false;
    time_stamp last_tick;
    double slit_debt = 0.0;     // fractional slits carried to the next tick

    static Params sanitized(Params q) {
      q.width  = std::max(q.width, 2);
      q.height = std::max(q.height, 2);
      q.slit_width   = std::max(q.slit_width, 1);
      q.slit_spacing = std::max(q.slit_spacing, 0);
      q.perspective_factor = std::max(q.perspective_factor, 1.0);
      q.slits_per_second   = std::clamp(q.slits_per_second, 0.0, 10000.0);
      return q;
    }

    void worker() {
      while (auto job = jobs.dequeue()) {
        if (clear_requested.exchange(false))
          wipe();

        switch (job->kind) {
        case Job::Kind::Configure:
          apply_params(sanitized(job->params));
          break;

        case Job::Kind::Frame: {
          // The same Mat again (a still image is sent on every tick)
          // is not a new frame. We hold a reference to the one we
          // have, so its pixels cannot have been freed and reused,
          // and the same address really is the same picture.
          SideState& s = side_state(job->side);
          if (job->frame.data != s.frame.data
              || job->frame.size() != s.frame.size()) {
            s.frame = job->frame;
            s.prepared_stale = true;
          }
          // Another gel: the slit starts again at the near edge,
          // and what it saw of the last gel is not blended in.
          if (job->gel != s.gel) {
            s.gel = job->gel;
            s.phase = 0.0;
            s.have_prev = false;
            s.prev_column.release();
            s.pass_reported = false;
          }
          break;
        }

        case Job::Kind::Tick:
          if (job->epoch != epoch.load())
            break; // from before playback was stopped
          if (ready) {
            integrate(job->ts);
            compose(job->ts);
          }
          break;

        case Job::Kind::Resume:
          have_last_tick = false;
          break;

        case Job::Kind::Clear:
          break; // done above; this job was only to wake us
        }
      }
    }

    // Back to how things were before the first frame.
    void wipe() {
      for (SideState* s : {&left, &right}) {
        s->frame.release();
        s->prepared.release();
        s->prepared_stale = true;
        s->have_prev = false;
        s->prev_column.release();
        s->gel = 0;
        s->pass_reported = false;
        s->phase = 0.0;
        if (!s->strip.empty()) s->strip.setTo(cv::Scalar::all(0));
      }
      slit_debt = 0.0;
      have_last_tick = false;
    }

    SideState& side_state(Side side) {
      return side == Side::Left ? left : right;
    }

    // The scan is always worked out as a vertical one. For a
    // horizontal scan that is done on the picture turned on its
    // side, so its width is the picture's height and the other
    // way about.
    bool lying_flat() const { return p.effect == ScanEffect::HorizontalSlit; }
    int scan_width() const  { return lying_flat() ? p.height : p.width; }
    int scan_height() const { return lying_flat() ? p.width : p.height; }

    void apply_params(const Params& q) {
      const bool geometry_changed = !ready
        || q.width  != p.width
        || q.height != p.height
        || q.effect != p.effect
        || q.slit_width   != p.slit_width
        || q.slit_spacing != p.slit_spacing;
      p = q;

      // from here on everything is in the scan's own terms, which
      // for a horizontal scan are the picture's turned on its side.
      const int half = scan_width() / 2;
      const double w = half;
      const double h = scan_height();

      if (geometry_changed) {
        for (SideState* s : {&left, &right}) {
          s->strip   = cv::Mat::zeros(scan_height(), half, CV_8UC3);
          s->scratch = cv::Mat::zeros(scan_height(), half, CV_8UC3);
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
      build_render_tables(half, scan_height());

      if (geometry_changed) {
        for (SideState* s : {&left, &right}) {
          s->prepared_stale = true;
          s->prev_column.release();
        }
      }
      ready = true;
    }

    // Take the perspective transform apart; see the tables above.
    void build_render_tables(int half, int high) {
      const cv::Matx33d inverse = cv::Matx33d(perspective).inv();

      // where a point of the picture comes from in the strip. Both
      // are in OpenCV's coordinates, where a pixel's centre is at
      // whole numbers.
      auto strip_point = [&](double x, double y) {
        const cv::Vec3d v = inverse * cv::Vec3d(x, y, 1.0);
        return cv::Point2d(v[0] / v[2], v[1] / v[2]);
      };
      const double mid_y = (high - 1) / 2.0;

      span_lo_x.create(high, half, CV_32FC1);
      span_hi_x.create(high, half, CV_32FC1);
      span_y.create(high, half, CV_32FC1);
      span_weight.create(high, half, CV_32FC3);

      for (int x = 0; x < half; ++x) {
        // The span of the strip that lands on this column: from
        // the left edge of the column to its right edge. The +0.5
        // moves us to coordinates where pixel i covers [i, i+1).
        double lo = strip_point(x - 0.5, mid_y).x + 0.5;
        double hi = strip_point(x + 0.5, mid_y).x + 0.5;
        if (lo > hi) std::swap(lo, hi);

        // never narrower than one pixel: a one pixel wide average
        // at a fractional position is plain linear interpolation.
        if (hi - lo < 1.0) {
          const double mid = (lo + hi) / 2.0;
          lo = mid - 0.5;
          hi = mid + 0.5;
        }
        lo = std::clamp(lo, 0.0, double(half) - 1.0);
        hi = std::clamp(hi, lo + 1.0, double(half));

        // cv::remap reads positions to the nearest 1/32 of a pixel.
        // Round the edges the same way ourselves, so that the width
        // we divide by is exactly the width that gets summed;
        // otherwise the columns come out unevenly bright.
        lo = std::round(lo * 32.0) / 32.0;
        hi = std::round(hi * 32.0) / 32.0;

        // the vertical stretch of this column, which is linear:
        // the rows of the strip that its top and bottom come from.
        const double top = strip_point(x, 0).y;
        const double bottom = strip_point(x, high - 1).y;
        const double per_row = (high > 1) ? (bottom - top) / (high - 1) : 0.0;
        const float weight = float(1.0 / (hi - lo));
        for (int y = 0; y < high; ++y) {
          span_lo_x.at<float>(y, x) = float(lo);
          span_hi_x.at<float>(y, x) = float(hi);
          span_y.at<float>(y, x) = float(top + per_row * y);
          span_weight.at<cv::Vec3f>(y, x) = cv::Vec3f(weight, weight, weight);
        }
      }
    }

    // The slit's position for a sweep phase, from 0 at the left of
    // the frame to 1 at the right. It runs across and back, one way
    // per unit of phase.
    double slit_u(double phase, bool sweeps) const {
      if (!sweeps || p.sweep_rate == 0.0)
        return 0.5;

      double t = std::fmod(phase, 2.0);
      if (t < 0.0) t += 2.0;
      return (t <= 1.0) ? t : 2.0 - t;
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

      add_slits(left, slits, p.sweep_left, p.sequence_left, pass_finished_left);
      add_slits(right, slits, p.sweep_right, p.sequence_right, pass_finished_right);
    }

    // Turn the frame the way the scan wants it, and scale it to the
    // height of the strip. Once per frame.
    // Shrinking averages over the rows it drops (INTER_AREA);
    // plain interpolation there would alias.
    void prepare(SideState& s) {
      const int high = s.strip.rows;
      if (!s.prepared_stale && s.prepared.rows == high)
        return;

      // for a horizontal scan the frame goes in on its side
      cv::Mat frame = s.frame;
      if (lying_flat())
        cv::transpose(s.frame, frame);

      if (frame.rows == high)
        s.prepared = frame;
      else
        cv::resize(frame, s.prepared, cv::Size(frame.cols, high), 0, 0
                   , (frame.rows > high) ? cv::INTER_AREA : cv::INTER_CUBIC);
      s.prepared_stale = false;
    }

    // The new slits, as one block of picture `block_w` wide with
    // the newest column at x = 0.
    //
    // Between the last tick and this one the slit went from x_prev
    // to x_cur across the frame (in pixels, where pixel i covers
    // [i, i+1)). The block is that stretch of the frame, resampled
    // to fit, so that neighbouring slits join up without a seam
    // whatever the sweep speed is. This is what replaces cutting
    // fixed pieces at rounded positions, which gave the jaggies.
    void swept_block(const cv::Mat& frame, double x_prev, double x_cur
                     , int block_w, cv::Mat& block) const {
      const int high = frame.rows;
      const double per_column = std::abs(x_cur - x_prev) / block_w;

      if (per_column <= 1.0) {
        // The frame is being stretched: interpolate. Column c of
        // the block looks at x_cur + (x_prev - x_cur) (c + 0.5) / w.
        const double a = (x_prev - x_cur) / block_w;
        const double b = x_cur + a * 0.5 - 0.5;
        const cv::Matx23d block_to_frame(a, 0.0, b
                                         , 0.0, 1.0, 0.0);
        cv::warpAffine(frame, block, block_to_frame, cv::Size(block_w, high)
                       , cv::INTER_LINEAR | cv::WARP_INVERSE_MAP
                       , cv::BORDER_REPLICATE);
      } else {
        // The frame is being squeezed: average, so that nothing
        // the slit passed over is skipped.
        int x0 = int(std::floor(std::min(x_prev, x_cur)));
        int x1 = int(std::ceil(std::max(x_prev, x_cur)));
        x0 = std::clamp(x0, 0, frame.cols - 1);
        x1 = std::clamp(x1, x0 + 1, frame.cols);
        cv::resize(frame(cv::Rect(x0, 0, x1 - x0, high)), block
                   , cv::Size(block_w, high), 0, 0, cv::INTER_AREA);
        if (x_cur > x_prev)
          cv::flip(block, block, 1); // newest goes at x = 0
      }
    }

    // The same for a slit that is standing still: there is nothing
    // to sweep over, so the block fades from what the slit saw at
    // the last tick to what it sees now.
    void faded_block(SideState& s, const cv::Mat& frame, double u
                     , int block_w, cv::Mat& block) const {
      const int high = frame.rows;

      // what the slit sees: slit_width columns of the frame, averaged
      const int aperture = std::min(p.slit_width, frame.cols);
      const int x0 = std::clamp(int(std::lround(u * frame.cols - aperture / 2.0))
                                , 0, frame.cols - aperture);
      cv::Mat column;
      if (aperture == 1)
        column = frame.col(x0).clone();
      else
        cv::resize(frame(cv::Rect(x0, 0, aperture, high)), column
                   , cv::Size(1, high), 0, 0, cv::INTER_AREA);

      if (s.prev_column.size() != column.size())
        s.prev_column = column;

      block.create(high, block_w, CV_8UC3);
      for (int c = 0; c < block_w; ++c) {
        const double now = 1.0 - (c + 0.5) / block_w; // 1 newest, 0 oldest
        cv::Mat target = block.col(c);
        cv::addWeighted(s.prev_column, 1.0 - now, column, now, 0.0, target);
      }
      s.prev_column = column;
    }

    void add_slits(SideState& s, int slits, bool sweeps
                   , bool sequence, std::atomic<bool>& pass_finished) {
      if (slits <= 0 || s.frame.empty())
        return;

      const int sw    = p.slit_width;
      const int pitch = sw + p.slit_spacing;
      const int wide  = s.strip.cols;
      const int high  = s.strip.rows;

      // no more slits than fit in the strip
      slits = std::min(slits, wide / pitch + 1);
      const int shift = std::min(slits * pitch, wide);

      // move what is there along by `shift`, into the scratch
      // image, blank the gap at the front, and swap.
      if (shift < wide) {
        s.strip(cv::Rect(0, 0, wide - shift, high))
          .copyTo(s.scratch(cv::Rect(shift, 0, wide - shift, high)));
      }
      s.scratch(cv::Rect(0, 0, shift, high)).setTo(cv::Scalar::all(0));
      std::swap(s.strip, s.scratch);

      // where the slit is now, and where it was at the last tick
      const double step = (p.slits_per_second > 0.0)
        ? p.sweep_rate / p.slits_per_second
        : 0.0;
      s.phase += slits * step;

      double u_cur = slit_u(s.phase, sweeps);

      // One gel of several: the slit goes across once and stops at
      // the far edge. There it tells the GUI, once, and waits for
      // the frames of the next gel, which start it off again.
      if (sequence && sweeps && p.sweep_rate != 0.0) {
        const double across = std::min(std::abs(s.phase), 1.0);
        u_cur = (p.sweep_rate > 0.0) ? across : 1.0 - across;
        if (across >= 1.0 && !s.pass_reported) {
          s.pass_reported = true;
          pass_finished = true;
          m_pass_dispatcher.emit();
        }
      }

      const double u_prev = s.have_prev ? s.prev_u : u_cur;
      s.prev_u = u_cur;
      s.have_prev = true;

      // all the new slits as one seamless block of picture
      prepare(s);
      const int block_w = slits * sw;
      cv::Mat block;
      if (u_prev != u_cur) {
        swept_block(s.prepared, u_prev * s.prepared.cols, u_cur * s.prepared.cols
                    , block_w, block);
        s.prev_column = block.col(0).clone();
      } else {
        faded_block(s, s.prepared, u_cur, block_w, block);
      }

      // and into the strip it goes, a slit at a time, leaving the
      // spacing between them. Slit 0 is the newest, at x = 0.
      for (int j = 0; j < slits; ++j) {
        const int x_dst = j * pitch;
        if (x_dst >= wide)
          break;
        const int w_dst = std::min(sw, wide - x_dst);
        block(cv::Rect(j * sw, 0, w_dst, high))
          .copyTo(s.strip(cv::Rect(x_dst, 0, w_dst, high)));
      }
    }

    // One half of the picture from a strip: the perspective
    // transform, filtered against aliasing.
    //
    // Towards the middle of the picture many columns of the strip
    // are squeezed into one. warpPerspective looks at just the 2 x 2
    // pixels where each point lands, so there it skips most of the
    // strip, and fine detail turns into jaggies and shimmer. Here
    // each pixel of the picture is instead the average of the whole
    // span of the strip that it covers.
    //
    // A running total along each row makes that cheap: the sum
    // over a span is the total at its right edge less the total at
    // its left, however wide the span is. Reading the totals
    // between pixels by linear interpolation makes it exact for
    // edges that fall inside a pixel, and where the span is a
    // single pixel wide it comes to plain linear interpolation.
    // The stretch up and down is done in the same reading.
    void render_half(const cv::Mat& strip, cv::Mat& out) const {
      const int high = strip.rows;

      // totals(y, x) is the sum of row y left of x. cv::integral
      // sums over everything above as well, so take the row above
      // away again. 32 bit sums do unless the picture is huge.
      const bool huge = double(strip.cols) * high * 255.0 > 2.0e9;
      cv::Mat integral;
      cv::integral(strip, integral, huge ? CV_64F : CV_32S);
      // (subtract first and convert to float after: the sums over
      // the whole picture are too big for a float to hold exactly,
      // the totals of one row are not.)
      cv::Mat totals;
      cv::subtract(integral.rowRange(1, high + 1), integral.rowRange(0, high)
                   , totals);
      totals.convertTo(totals, CV_32F);

      cv::Mat at_lo, at_hi;
      cv::remap(totals, at_lo, span_lo_x, span_y, cv::INTER_LINEAR, cv::BORDER_REPLICATE);
      cv::remap(totals, at_hi, span_hi_x, span_y, cv::INTER_LINEAR, cv::BORDER_REPLICATE);

      cv::subtract(at_hi, at_lo, at_hi);
      cv::multiply(at_hi, span_weight, at_hi);
      at_hi.convertTo(out, CV_8U);
    }

    // Build the output frame from the two strips and hand it over.
    void compose(time_stamp tick) {
      const int wide = scan_width();
      const int high = scan_height();
      const int half = wide / 2;
      const SideState* r = &right;
      const SideState* l = &left;
      if (p.mirror_missing_side) {
        if (right.frame.empty() && !left.frame.empty()) r = &left;
        if (left.frame.empty() && !right.frame.empty()) l = &right;
      }

      // always a fresh Mat; the GUI may still be painting the last one.
      cv::Mat out = cv::Mat::zeros(high, wide, CV_8UC3);
      cv::Mat warped;

      if (!r->frame.empty()) {
        render_half(r->strip, warped);
        warped.copyTo(out(cv::Rect(wide - half, 0, half, high)));
      }

      if (!l->frame.empty()) {
        if (l != r || warped.empty())
          render_half(l->strip, warped);
        cv::Mat mirrored;
        cv::flip(warped, mirrored, 1);
        mirrored.copyTo(out(cv::Rect(0, 0, half, high)));
      }

      // a horizontal scan was made on its side: stand it up again.
      // Left becomes top and right becomes bottom.
      if (lying_flat()) {
        cv::Mat upright;
        cv::transpose(out, upright);
        out = upright;
      }

      recorder.write(out); // does nothing unless we are recording

      {
        lockguard lock(out_mutex);
        out_frame = out;
        out_ts = tick;
      }
      m_dispatcher.emit();
    }
  };
}
