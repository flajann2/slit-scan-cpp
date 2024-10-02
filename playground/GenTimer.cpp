
#include <chrono>
#include <thread>
#include <atomic>
#include <iostream>
#include <functional>

class HighPrecisionTimer {
private:
  std::atomic<bool> running{false};
  std::thread timerThread;
  std::chrono::nanoseconds interval;
  std::function<void()> callback;
  
  void timerLoop() {
    auto nextTick = std::chrono::steady_clock::now() + interval;
    while (running) {
      std::this_thread::sleep_until(nextTick);
      if (running) {
        callback();
        nextTick += interval;
      }
    }
  }
  
public:
  HighPrecisionTimer(std::chrono::nanoseconds interval
                     , std::function<void()> callback)
    : interval(interval), callback(callback) {}
  
  ~HighPrecisionTimer() {
    stop();
  }
  
  void start() {
    if (!running.exchange(true)) {
      timerThread = std::thread(&HighPrecisionTimer::timerLoop, this);
    }
  }
  
  void stop() {
    running = false;
    if (timerThread.joinable()) {
      timerThread.join();
    }
  }
};

int main() {
  int counter = 0;
  auto callback = [&counter]() {
    std::cout << "Timer tick: " << ++counter << std::endl;
  };
  
  HighPrecisionTimer timer(std::chrono::milliseconds(50), callback);
  timer.start();
  
  // Let the timer run for 5 seconds
  std::this_thread::sleep_for(std::chrono::seconds(5));
  
  timer.stop();
  return 0;
}
