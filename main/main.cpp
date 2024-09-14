// SlitScan main

#include <main.h>
#include <gui.h>
//#include <hekate/hekate>

using namespace std;

SlitScanApp SlitScanApp::ss_app;

int main(int argc, char* argv[]) {
  return SlitScanApp::ss_app.run(argc, argv);
}
