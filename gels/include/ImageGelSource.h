// ImageGelSource.h -- GelSource for images
#pragma once

namespace Gel {
  using namespace std;
  
  const auto default_frame_rate = micros(1000000/30); 
  
  // One still image as a gel. On every tick it broadcasts the image,
  // as it is. Its rotation, spin and flip go with it by way of
  // pose_at(), and the sink does the turning and the slit work (see
  // SlitScan.h).
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
    }

    const OneGel& get_gel() const { return gel; }

    cv::Mat preview_frame() override { return gel.frame; }

  protected:
    void on_tick(time_stamp tick) {
      if (!is_active() || gel.frame.empty())
        return;

      // always the very same Mat, so the scanner can tell that the
      // picture has not changed.
      broadcast_next_frame(gel.frame, tick);
    }
    
  private:
    OneGel gel;
  };
}
