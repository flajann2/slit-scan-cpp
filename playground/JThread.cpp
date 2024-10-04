// TODO: All of these includes are
// TODO: not needed. Cull.

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

// to avoid conflit with the applican class, we pull this in
// sepreatly.
#include <GelQueue.h>

using namespace std;

const int tries = 100000;
const int pause_mil = 1;
const int time_out = 4;

Gel::GelQueue<int> gq;

void t1() {
  for (int i = 0; !gq.is_stopped() && i < tries; ++i) {
    gq.enqueue(i);
    cout << "t1(" << i << ")" << endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(pause_mil));
  }
}

void t2() {
  for (int i = 0; !gq.is_stopped() && i > -tries; --i) {
    gq.enqueue(i);
    cout << "t2(" << i << ")" << endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(pause_mil));
  }
}

void wto() {
  std::this_thread::sleep_for(std::chrono::seconds(time_out));
  gq.shutdown();
  cout << "wto timeout after " << time_out << " seconds" << endl;
}

int main() {
  std::jthread wt1(&t1);
  std::jthread wt2(&t2);
  std::jthread timeout_thread(&wto);

  while (!gq.is_stopped()) {
    cout << "t->" << gq.dequeue().value_or(0) << endl;
  }
  cout << "shutting down." << endl;
  
  //  std::this_thread::sleep_for(std::chrono::seconds(2));
  return 0;
}
