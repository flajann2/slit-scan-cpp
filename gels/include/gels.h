// Gels
#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
// OpenCV 5 moved getRotationMatrix2D, getPerspectiveTransform and
// friends out of imgproc into the new geometry module, and
// opencv.hpp does not pull that in. OpenCV 4 has no such header.
#if __has_include(<opencv2/geometry.hpp>)
#include <opencv2/geometry.hpp>
#endif
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
#include <cmath>
#include <string>

namespace Gel {
  using namespace std;
  
  // steady_clock, not system_clock: the wall clock can be stepped
  // (NTP, DST, the user), which would stall the ticks or fire a
  // burst of them. steady_clock only ever moves forward.
  using time_basis = std::chrono::steady_clock;
  using time_stamp = std::chrono::time_point<time_basis>;
  using micros = std::chrono::microseconds;
  using lockguard = std::lock_guard<std::mutex>;
  using sig_next_tick = sigc::signal<void(time_stamp)>;
  using sig_next_frame = sigc::signal<void(const cv::Mat&, time_stamp)>;

  enum class ScanMeth { Left, Right, Top, Bottom, Circular };

  // seconds between two time stamps, as a double
  inline double seconds_between(time_stamp from, time_stamp to) {
    return std::chrono::duration<double>(to - from).count();
  }

  class MasterTickServer {
    friend class IMasterTickServer;

  public:
    MasterTickServer() {
      m_dispatcher.connect(sigc::mem_fun(*this
                                         , &MasterTickServer::internal_tick));
    }

    // the timer thread must be gone before our members are.
    ~MasterTickServer() { stop(); }

    static MasterTickServer master_tick_server; 

  private:

    Glib::Dispatcher m_dispatcher; // internal tick server will use this
    sig_next_tick m_sig_next_tick;
    std::atomic<bool> running{false};
    std::thread timer_thread;
    std::atomic<micros> interval{micros(1000000/30)};

    // m_mutex guards m_pending, which is the only state shared
    // between the timer thread and the GUI thread.
    std::mutex m_mutex;
    std::queue<time_stamp> m_pending;
    
    // send out the internal ticks to the attached slots.
    // Runs in the GUI thread, by way of the dispatcher.
    void internal_tick() {
      time_stamp tick;
      {
        lockguard lock(m_mutex);
        if (m_pending.empty()) return;
        tick = m_pending.front();
        m_pending.pop();
      }
      m_sig_next_tick.emit(tick);
    }

    // Runs in the timer thread. next_tick is local to this thread,
    // so it needs no locking.
    void time_frame_stamps() {
      time_stamp next_tick = time_basis::now() + interval.load();
      
      while (running) {
        std::this_thread::sleep_until(next_tick);
        if (!running) break;

        {
          lockguard lock(m_mutex);
          m_pending.push(next_tick);
        }
        m_dispatcher.emit();

        next_tick += interval.load();

        // if we have fallen behind (machine suspended, debugger,
        // heavy load), resynchronise rather than firing a burst.
        const auto now = time_basis::now();
        if (next_tick < now)
          next_tick = now + interval.load();
      }
    }
    
  protected:
    // Start the internal tick server
    void start() {
      if (!running.exchange(true)) {
        if (timer_thread.joinable()) timer_thread.join();
        timer_thread = std::thread(&MasterTickServer::time_frame_stamps, this);
      }
    }

    // Stop the internal tick server
    void stop() {
      running = false;
      if (timer_thread.joinable()) {
        timer_thread.join();
      }
      // ticks that were queued but not yet delivered are stale now
      lockguard lock(m_mutex);
      m_pending = {};
    }

    bool is_running() { return running; }

    // Slots connected here are called in the GUI thread.
    sig_next_tick& signal_tick() { return m_sig_next_tick; }
  };

  MasterTickServer& obtain_master_tick_server();
  
  class IMasterTickServer {
  private:
    MasterTickServer& mts;
    
  public:
    IMasterTickServer() : mts(obtain_master_tick_server()) { }

    void start() { mts.start(); }
    void stop() { mts.stop(); }
    bool is_running() { return mts.is_running(); }
    
    sig_next_tick& signal_tick() { return mts.signal_tick(); }
    void set_interval(micros interval) { mts.interval = interval; }

    static unique_ptr<IMasterTickServer> create() {
      return make_unique<IMasterTickServer>();
    }
  };

  // defined in Frame2Canvas.h
  inline void rotate(const cv::Mat& frame, cv::Mat& rotated, double angle);

  // Base of all gel sources. A source does image processing on its
  // frames and broadcasts them; it knows nothing about slits.
  //
  // The tick server is shared by the whole application, so a source
  // neither starts nor stops it, nor sets its interval -- whoever
  // owns playback does that. sigc::trackable makes our signal
  // connections go away with us.
  class GelSource : public IMasterTickServer
                  , public sigc::trackable {
  public:
    // what is done to every frame before it is broadcast
    struct Transform {
      double stationary_rotation = 0.0; // degrees
      double rotation_rate = 0.0;       // rotations per second
      bool flip = false;                // mirror left-right
    };

    GelSource() = default;
    virtual ~GelSource() = default;
    
    sig_next_frame& signal_next_frame() { return m_sig_next_frame;  }

    // an inactive source ignores the ticks and broadcasts nothing.
    void set_active(bool active_) {
      if (active_ == active)
        return;
      active = active_;
      if (active)
        on_activated();
      else
        on_deactivated();
    }
    bool is_active() const { return active; }

    // the transformations, as set in the gel's configuration dialog.
    //
    // The spin runs on the clock, whether or not anything is
    // playing, so that the dialog and the scan show the same angle.
    // Changing the rate carries on from the angle reached so far;
    // the picture does not jump.
    virtual void set_transform(double stationary_rotation
                               , double rotation_rate
                               , bool flip) {
      const auto now = time_basis::now();
      spin_anchor = spin_at(now);
      spin_anchor_time = now;
      transform = Transform{stationary_rotation, rotation_rate, flip};
    }
    const Transform& get_transform() const { return transform; }

    bool is_spinning() const { return transform.rotation_rate != 0.0; }

    // the rotation, in degrees, at a point in time: the stationary
    // rotation and however far the spin has got.
    double angle_at(time_stamp when) const {
      return std::fmod(transform.stationary_rotation + spin_at(when), 360.0);
    }
    double angle_now() const { return angle_at(time_basis::now()); }

    // For the configuration dialog: the latest frame as it comes,
    // before any transformation. Empty if there is none to show.
    virtual cv::Mat preview_frame() { return {}; }

    // The dialog says when it is watching. Sources that have to be
    // running to have frames to show (videos, cameras) run for it.
    virtual void set_previewing(bool) {}

    // The frame with the transformations applied, as of `when`.
    // Hands back the very same frame if there is nothing to do, and
    // otherwise always a fresh Mat, never touching the one passed in.
    cv::Mat transformed(const cv::Mat& frame, time_stamp when) const {
      const double angle = angle_at(when);
      if (angle == 0.0 && !transform.flip)
        return frame;

      cv::Mat out;
      if (transform.flip)
        cv::flip(frame, out, 1);
      else
        out = frame;

      if (angle != 0.0) {
        cv::Mat rotated;
        rotate(out, rotated, angle);
        out = rotated;
      }
      return out;
    }

    // Do the frames change by themselves over time? True for videos
    // and cameras, false for still images. The scanner holds its
    // slit still for the former and sweeps it across the latter.
    virtual bool is_moving() const { return false; }

    // T Must be a derivate of GelSource
    template <typename T, typename... Args>
    static unique_ptr<GelSource> create(Args&&... args) {
      return make_unique<T>(std::forward<Args>(args)...);
    }
    
  protected:
    // derived classes will use this to broadcast frames. The frame
    // must not be modified after it has been broadcast, since the
    // receivers may hold on to it (cv::Mat shares its pixels).
    virtual void broadcast_next_frame(const cv::Mat& frame, time_stamp timestamp) {
      m_sig_next_frame.emit(frame, timestamp);
    }

    // called when the source goes from inactive to active, and back
    virtual void on_activated() {}
    virtual void on_deactivated() {}

    // how far the spin has got, in degrees
    double spin_at(time_stamp when) const {
      return std::fmod(spin_anchor
                       + 360.0 * transform.rotation_rate
                               * seconds_between(spin_anchor_time, when)
                       , 360.0);
    }

  private:
    sig_next_frame m_sig_next_frame;
    Transform transform;
    double spin_anchor = 0.0;  // degrees of spin at spin_anchor_time
    time_stamp spin_anchor_time = time_basis::now();
    bool active = false;
  };
}

#include <GelQueue.h>
#include <Recorder.h>
#include <Frame2Canvas.h>
#include <ImageGelSource.h>
#include <VideoGelSource.h>
#include <CameraGelSource.h>
#include <SlitScan.h>
