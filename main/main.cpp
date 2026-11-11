// SlitScan main

#include <main.h>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <Builder.h>
//#include <hekate/hekate>

using namespace std;

extern "C" GResource *gresource_get_resource (void);

SSA SSA::ss_app;

namespace Gui {
  GResource* resource = nullptr;
  shared_ptr<Gtk::Builder> app_builder;

  shared_ptr<Gtk::Builder> obtain_new_builder() {
    if (!resource)
      resource = gresource_get_resource(); // this loads and "registers" the resouce by way of PFM!

    auto builder = Gtk::Builder::create_from_resource(ss::resource + "slitscan_main.ui");
    builder->add_from_resource(ss::resource + "slitscan_main_settings.ui");
    builder->add_from_resource(ss::resource + "file_gel_select.ui");
    builder->add_from_resource(ss::resource + "gel_config.ui");
    builder->add_from_resource(ss::resource + "video_config.ui");
    return builder;
  }

  shared_ptr<Gtk::Builder> obtain_builder() {
    if (!app_builder) {
      app_builder = obtain_new_builder();
    }
    return app_builder;
  }

  // static and therefore "global" configurations.
  MainSettings::Settings MainSettings::main_settings;
  VideoConfig::Configuration VideoConfig::video_configuration;
}

namespace Gel {
  MasterTickServer MasterTickServer::master_tick_server;
  MasterTickServer& obtain_master_tick_server() { return MasterTickServer::master_tick_server; }
}

// what the process exits with if the tidying up at the end hangs
constexpr int result_when_stuck = 3;

int main(int argc, char* argv[]) {
  const int result = SSA::ss_app.run(argc, argv);

  // The window is closed. What remains is tidying up: stopping the
  // threads, closing the camera, finishing a recording. All of
  // that normally takes a moment. But should any of it get stuck,
  // the process must not live on with no window to show for it,
  // so it is given ten seconds and then shown the door.
  std::thread([] {
    std::this_thread::sleep_for(std::chrono::seconds(10));
    std::cerr << "SlitScan: still tidying up ten seconds after the window "
                 "closed; leaving anyway." << std::endl;
    std::_Exit(result_when_stuck);
  }).detach();

  return result;
}
