// ImageGelSource.h -- GelSource for images
#pragma once

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
      double stationary_rotation = 0.0; // degrees
      double rotation_rate = 0.0;       // rotations per second      
    };

    void set_gel(const OneGel& gel_) {
      gel = gel_;
      cached.release();
      GelSource::set_transform(gel.stationary_rotation
                               , gel.rotation_rate
                               , get_transform().flip);
    }

    void set_transform(double stationary_rotation
                       , double rotation_rate
                       , bool flip) override {
      GelSource::set_transform(stationary_rotation, rotation_rate, flip);
      gel.stationary_rotation = stationary_rotation;
      gel.rotation_rate = rotation_rate;
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

      // a stationary transformation only has to be computed once.
      if (!is_spinning()) {
        if (cached.empty())
          cached = transformed(gel.frame, 0.0);
        broadcast_next_frame(cached, tick);
        return;
      }

      broadcast_next_frame(transformed(gel.frame, seconds_between(start_time, tick))
                           , tick);
    }
    
  private:
    OneGel gel;
    cv::Mat cached;   // transformed frame, when it does not change over time
    bool started = false;
    time_stamp start_time;
  };
}
