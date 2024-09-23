#include <iostream>
#include <thread>
#include <functional>
#include <atomic>
#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue {
public:
  void push(T ob) {
    std::lock_guard<std::mutex> lock(mtx);
    queue.push(ob);
    cv.notify_one();
  }

  T pop() {
    std::lock_guard<std::mutex> lock(mtx);
    if (queue.empty()) return nullptr;
    auto func = queue.front();
    queue.pop();
    return func;
  }

  bool empty() {
    std::lock_guard<std::mutex> lock(mtx);
    return queue.empty();
  }

  // TODO -- replace this by something a little less ugly
  std::condition_variable& cond() { return cv; } 
  
private:
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cv;
};

class Worker {
public:
    Worker() : stop(false), workerThread(&Worker::process, this) {}

    ~Worker() {
        stop = true;
        taskQueue.cond().notify_all();
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }

    void enqueue(std::function<void()> func) {
        taskQueue.push(func);
    }

private:
    void process() {
        while (!stop) {
            while (taskQueue.empty() && !stop) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            while (!taskQueue.empty()) {
                auto task = taskQueue.pop();
                if (task) task(); // Execute the task
            }
        }
    }

  ThreadSafeQueue<std::function<void()>> taskQueue;
  std::jthread workerThread; // Using jthread for automatic joining
  std::atomic<bool> stop;
};

// Example usage of the Worker class
int main() {
  Worker worker;

  // Simulate emitting signals by enqueuing tasks
  for (int i = 0; i < 15; ++i) {
    worker.enqueue([i]() {
      std::cout << "Processing task " << i + 1 << " in thread: "
                << std::this_thread::get_id() << "\n";
      std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate work
    });
  }

  // Give some time for tasks to be processed
  std::this_thread::sleep_for(std::chrono::seconds(6));
  
  return 0;
}
