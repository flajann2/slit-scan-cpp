// ImageGel.h -- GelSource for images
#pragma once

#include <ctime>
#include <iomanip>

namespace Gel {
  using namespace std;
  
  const auto default_frame_rate = micros(1000000/30); 
  
  class ImageGel : public GelSource {
  public:
    ImageGel(micros interval = default_frame_rate)
      : GelSource(interval, [&](time_stamp ts) { next_frame(ts); }) {
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
    void next_frame(time_stamp ts) {
      cout << "next_frame() " << ts << endl;
    }
    
  private:
  };

}
