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

    // a spinning image is turned afresh on every tick, in the GUI
    // thread, so it is first brought down to this many pixels on
    // its longer side. The scanner scales it to its own height
    // anyway, and nothing bigger than this is ever needed there.
    static constexpr int spin_size = 2160;

    void set_gel(const OneGel& gel_) {
      gel = gel_;
      cached.release();
      spin_base.release();
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

    cv::Mat preview_frame() override { return gel.frame; }

  protected:
    void on_tick(time_stamp tick) {
      if (!is_active() || gel.frame.empty())
        return;

      // a stationary transformation only has to be computed once.
      if (!is_spinning()) {
        if (cached.empty())
          cached = transformed(gel.frame, tick);
        broadcast_next_frame(cached, tick);
        return;
      }

      // the spin goes by the clock, not by a count of ticks, so a
      // change of frame rate smooths rather than speeds up.
      if (spin_base.empty()) {
        const int longer = std::max(gel.frame.cols, gel.frame.rows);
        if (longer > spin_size) {
          const double scale = double(spin_size) / longer;
          cv::resize(gel.frame, spin_base, cv::Size(), scale, scale, cv::INTER_AREA);
        } else {
          spin_base = gel.frame;
        }
      }
      broadcast_next_frame(transformed(spin_base, tick), tick);
    }
    
  private:
    OneGel gel;
    cv::Mat cached;    // transformed frame, when it does not change over time
    cv::Mat spin_base; // the image at a size that can be turned every tick
  };
}
