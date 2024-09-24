// Gels
#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>
#include <functional>
#include <atomic>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace Gel {
  class GelSource {
  public:
    virtual void signal_next_frame() = 0;
    
  protected:
    virtual void broadcast_next_frame(cv::Mat frame) {
    }
  };
}

#include <GelQueue.h>
