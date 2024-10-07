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
  using lockguard = std::lock_guard<std::mutex>;
  using sig_next_tick = sigc::signal<void(time_stamp)>;
  using sig_next_frame = sigc::signal<void(const cv::Mat&, time_stamp)>;

  enum class ScanMeth { Left, Right, Top, Bottom, Circular };
  
  class GelSource {
  public:
    GelSource(micros interval_
              , std::function<void(time_stamp)> tick_func_)
      : interval(interval_)
      , tick_func(tick_func_) {
      m_dispatcher.connect(sigc::mem_fun(*this, &GelSource::internal_tick));
    }
    
    ~GelSource() { stop(); }
    
    sig_next_frame signal_next_frame() { return m_sig_next_frame;  }
    
    // Start the internal tick server
    void start() {
      if (!running.exchange(true)) {
        timer_thread = std::thread(&GelSource::time_frame_stamps, this);
      }
    }

    // Stop the internal tick server
    void stop() {
      running = false;
      if (timer_thread.joinable()) {
        timer_thread.join();
      }
    }

    // T Must be a derivate of GelSource
    template <typename T>
    static unique_ptr<GelSource> create() { return make_unique<T>(); }

    // direct thread-safe access to tick server
    sig_next_tick& signal_tick() { return m_sig_next_tick; }
    
  protected:
    // derived classes will use this to broadcast frames
    virtual void broadcast_next_frame(cv::Mat frame, time_stamp timestamp) {
      m_sig_next_frame.emit(frame, timestamp);
    }


  private:
    std::mutex m_mutex;
    sig_next_frame m_sig_next_frame;
    sig_next_tick m_sig_next_tick;
    Glib::Dispatcher m_dispatcher; // internal tick server will use this
    
    std::atomic<bool> running{false};
    std::thread timer_thread;
    std::atomic<micros> interval;
    std::function<void(time_stamp)> tick_func;
    time_stamp m_current_tick;
    time_stamp m_next_tick;

    void time_frame_stamps() {
      {
        lockguard lock(m_mutex);
        m_current_tick = time_basis::now(); 
        m_next_tick = m_current_tick + interval.load();
      }
      
      while (running) {
        std::this_thread::sleep_until(m_next_tick);
        if (running) {
          m_current_tick = m_next_tick;
          m_dispatcher.emit();
          tick_func(m_current_tick);
          {
            lockguard lock(m_mutex);
            m_next_tick += interval.load();
          }
        }
      }
    }

    // send out the internal ticks to the attached slots
    void internal_tick() {
      m_sig_next_tick.emit(m_current_tick);
    }
  };
}

#include <GelQueue.h>
#include <Frame2Canvas.h>
#include <ImageGelSource.h>
#include <VideoGelSource.h>
#include <CameraGelSource.h>
