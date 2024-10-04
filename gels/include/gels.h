// Gels
#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
#include <sigc++/sigc++.h>
#include <thread>
#include <functional>
#include <generator>
#include <atomic>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <tuple>

namespace Gel {
  using namespace std;
  
  using time_basis =std::chrono::system_clock; // TODO: make this steady_clock 
  using time_stamp =std::chrono::time_point<time_basis>;
  using micros = std::chrono::microseconds;

  enum class ScanMeth { Left, Right, Top, Bottom, Circular };
  
  class GelSource {
  public:
    GelSource(micros  interval
              , std::function<void(time_stamp)> callback_)
      : interval(interval)
      , callback(callback_) {}
    
    ~GelSource() { stop(); }
    
    using sig_next_frame = sigc::signal<void(const cv::Mat&, int)>;
    sig_next_frame signal_next_frame() { return m_sig_next_frame;  }
    
    void start() {
      if (!running.exchange(true)) {
        timerThread = std::thread(&GelSource::time_frame_stamps, this);
      }
    }
  
    void stop() {
      running = false;
      if (timerThread.joinable()) {
        timerThread.join();
      }
    }

    // T Must be a derivate of GelSource
    template <typename T>
    static unique_ptr<GelSource> create() { return make_unique<T>(); }
    
  protected:
    virtual void broadcast_next_frame(cv::Mat frame, int timestamp) {
      m_sig_next_frame.emit(frame, timestamp);
    }

  private:
    sig_next_frame m_sig_next_frame;    

    std::atomic<bool> running{false};
    std::thread timerThread;
    micros interval;
    std::function<void(time_stamp)> callback;

    void time_frame_stamps() {
      auto next_tick = time_basis::now() + interval;
      while (running) {
        std::this_thread::sleep_until(next_tick);
        if (running) {
          callback(next_tick);
          next_tick += interval;
        }
      }
    }
  };
}

#include <GelQueue.h>
#include <Frame2Canvas.h>
#include <ImageGelSource.h>
#include <VideoGelSource.h>
#include <CameraGelSource.h>
