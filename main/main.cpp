// SlitScan main

#include <main.h>
//#include <hekate/hekate>

using namespace std;

SSA SSA::ss_app;

int main(int argc, char* argv[]) {
  return SSA::ss_app.run(argc, argv);
}
