// GelQueue.h of SlitScan -- thred-safe queuing
// for gel processing
#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <optional>
#include <queue>

namespace Gel {
  // Bounded, thread-safe queue.
  //
  // One mutex guards the queue, and two condition variables
  // signal the two things a thread can wait for: "not empty"
  // for consumers and "not full" for producers. Since there is
  // only the one mutex, there is no lock ordering to get wrong,
  // and so no deadly embrace.
  template <typename T> class GelQueue {

  protected:
    // caller must hold mtx
    inline bool _full() const {
      return queue.size() >= max_size;
    }

  public:
    GelQueue(std::size_t max_size_ = 100) : max_size(max_size_)
                                          , stop(false)
                                          , finishing(false) {}

    // Blocks while the queue is full. Returns false -- and drops
    // the object -- if the queue has been shut down.
    bool enqueue(T ob) {
      std::unique_lock<std::mutex> lock(mtx);
      not_full.wait(lock, [&](){ return stop || finishing || !_full(); });
      if (stop || finishing) return false;

      queue.push(std::move(ob));
      lock.unlock();
      not_empty.notify_one();
      return true;
    }

    // Never blocks. Returns false if the queue is full or shut
    // down. Use this from threads that must not stall, such as
    // the GUI thread.
    bool try_enqueue(T ob) {
      std::unique_lock<std::mutex> lock(mtx);
      if (stop || finishing || _full()) return false;

      queue.push(std::move(ob));
      lock.unlock();
      not_empty.notify_one();
      return true;
    }

    // Blocks while the queue is empty. Returns an empty optional
    // once the queue has been shut down, or has been finished and
    // emptied.
    std::optional<T> dequeue() {
      std::unique_lock<std::mutex> lock(mtx);
      not_empty.wait(lock, [&](){ return stop || finishing || !queue.empty(); });
      if (stop || queue.empty()) return {};

      auto ob = std::move(queue.front());
      queue.pop();
      lock.unlock();
      not_full.notify_one();
      return ob;
    }

    bool empty() {
      std::unique_lock<std::mutex> lock(mtx);
      return queue.empty();
    }

    bool full() {
      std::unique_lock<std::mutex> lock(mtx);
      return _full();
    }

    std::size_t size() {
      std::unique_lock<std::mutex> lock(mtx);
      return queue.size();
    }

    // The gentle way to end: nothing more is accepted, but what
    // is already queued can still be taken. Once that is gone,
    // dequeue() returns empty. Wakes up every blocked producer.
    void finish() {
      {
        std::unique_lock<std::mutex> lock(mtx);
        finishing = true;
      }
      not_empty.notify_all();
      not_full.notify_all();
    }

    // The abrupt way: what is queued is abandoned. Wakes up every
    // blocked producer and consumer.
    void shutdown() {
      {
        // taking the lock here closes the window where a thread
        // has checked `stop` but has not yet gone to sleep.
        std::unique_lock<std::mutex> lock(mtx);
        stop = true;
      }
      not_empty.notify_all();
      not_full.notify_all();
    }

    bool is_stopped(){ return stop; }

  private:
    std::size_t max_size;
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable not_empty;
    std::condition_variable not_full;
    std::atomic<bool> stop;
    bool finishing; // guarded by mtx
  };
}
