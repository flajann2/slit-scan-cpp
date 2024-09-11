// Gels
#pragma once

#include <opencv2/opencv.hpp>

class GelSource {
public:
  virtual void signal_next_frame() = 0;

protected:
  virtual void broadcast_next_frame(cv::Mat frame) {
  }
};
