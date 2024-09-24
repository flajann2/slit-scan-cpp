// GelQueue.h of SlitScan -- thred-safe queuing
// for gel processing
#pragma once


namespace Gel {
  template <typename T> class GelQueue {
  public:
    GelQueue(int max_size_ = 100) : max_size(max_size_)
                                  , stop(false) {}
    
    void enqueue(T ob) {
      std::unique_lock<std::mutex> lock(mtx);
      queue.push(ob);
      lock.unlock();
      cv.notify_one();
    }
    
    std::optional<T> dequeue() {
      std::unique_lock<std::mutex> lock(mtx);
      cv.wait(lock, [&](){ return is_stopped() || !queue.empty(); });
      if (is_stopped()) return {};
      
      auto ob = queue.front();
      queue.pop();
      return ob;
    }
    
    bool empty() {
      std::unique_lock<std::mutex> lock(mtx);
      return queue.empty();
    }

    void shutdown() {
      stop = true;
      cv.notify_all();
    }

    bool is_stopped(){ return stop; }
    
    // TODO -- replace this by something a little less ugly
    std::condition_variable& cond() { return cv; } 
    
  private:
    int max_size;
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> stop;
  };
}
