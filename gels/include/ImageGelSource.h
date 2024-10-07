// ImageGelSource.h -- GelSource for images
#pragma once

#include <ctime>
#include <iomanip>

namespace Gel {
  using namespace std;
  
  const auto default_frame_rate = micros(1000000/30); 
  
  class ImageGelSource : public GelSource {
  public:
    ImageGelSource(micros interval = default_frame_rate)
      : GelSource(interval)  {
      signal_tick().connect(sigc::mem_fun(*this, &ImageGelSource::on_tick));
      start(); // TODO:: dont start here     
    }

    struct OneGel {
      cv::Mat frame;
     
      string name; // for now, the pathname
      time_stamp load_time; // supposed to be a unique identifier. 
      
      ScanMeth meth;
      double stationary_rotation;
      double rotation_rate; // rotations per second      
    };

  protected:
    void on_tick(time_stamp tick) {
      cout << "ImageGelSource tick: " << tick << endl;      
    }
    
  private:
  };

}
