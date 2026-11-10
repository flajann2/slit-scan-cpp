// ImageGelSource.h -- GelSource for images
#pragma once

#include <cmath>

namespace Gel {
  using namespace std;

  const auto default_frame_rate = micros(1000000/30);

  // One still image as a gel. On every tick it broadcasts the image
  // with its transformations applied. The slit work is not done
  // here; that is the job of the sink (see SlitScan.h).
  class ImageGelSource : public GelSource {
  public:
    ImageGelSource() {
      signal_tick().connect(sigc::mem_fun(*this, &ImageGelSource::on_tick));
    }

    struct OneGel {
      cv::Mat frame;        // RGB, never modified once set

      string name; // for now, the pathname
      time_stamp load_time; // supposed to be a unique identifier.

      ScanMeth meth = ScanMeth::Left;
      bool flip = false;                // mirror left-right
      double stationary_rotation = 0.0; // degrees
      double rotation_rate = 0.0;       // rotations per second
    };

    void set_gel(const OneGel& gel_) {
      gel = gel_;
      cached.release();
    }

    // the transformations, as set in the gel's configuration dialog.
    void set_transform(double stationary_rotation
                       , double rotation_rate
                       , bool flip) {
      gel.stationary_rotation = stationary_rotation;
      gel.rotation_rate = rotation_rate;
      gel.flip = flip;
      cached.release();
    }

    const OneGel& get_gel() const { return gel; }

  protected:
    void on_activated() override { started = false; }

    void on_tick(time_stamp tick) {
      if (!is_active() || gel.frame.empty())
        return;

      // animations are based on elapsed time, not on a count of
      // ticks, so a change of frame rate smooths rather than speeds up.
      if (!started) {
        start_time = tick;
        started = true;
      }
      const double elapsed = seconds_between(start_time, tick);
      const double angle = std::fmod(gel.stationary_rotation
                                     + 360.0 * gel.rotation_rate * elapsed
                                     , 360.0);
      const bool spinning = gel.rotation_rate != 0.0;

      // nothing to do to the image at all: hand out the original.
      if (!spinning && angle == 0.0 && !gel.flip) {
        broadcast_next_frame(gel.frame, tick);
        return;
      }

      // a stationary transformation only has to be computed once.
      if (!spinning && !cached.empty()) {
        broadcast_next_frame(cached, tick);
        return;
      }

      // always into a fresh Mat: earlier frames may still be in
      // use by whoever received them.
      cv::Mat out;
      if (gel.flip)
        cv::flip(gel.frame, out, 1);
      else
        out = gel.frame;

      if (angle != 0.0) {
        cv::Mat rotated;
        rotate(out, rotated, angle);
        out = rotated;
      }

      if (!spinning)
        cached = out;
      broadcast_next_frame(out, tick);
    }

  private:
    OneGel gel;
    cv::Mat cached;   // transformed frame, when it does not change over time
    bool started = false;
    time_stamp start_time;
  };
}
