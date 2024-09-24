#include <gels.h>
#include <chrono>

using namespace std;

const int tries = 100000;
const int pause_mil = 2;
const int time_out = 4;

Gel::GelQueue<int> gq;

void t1() {
  for (int i = 0; !gq.is_stopped() && i < tries; ++i) {
    gq.enqueue(i);
    std::this_thread::sleep_for(std::chrono::milliseconds(pause_mil));
  }
}

void t2() {
  for (int i = 0; !gq.is_stopped() && i > -tries; --i) {
    gq.enqueue(i);
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
