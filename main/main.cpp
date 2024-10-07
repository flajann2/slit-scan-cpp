// SlitScan main

#include <main.h>
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
    return builder;
  }

  shared_ptr<Gtk::Builder> obtain_builder() {
    if (!app_builder) {
      app_builder = obtain_new_builder();
    }
    return app_builder;
  }

  MainSettings::Settings MainSettings::main_settings;
}

namespace Gel {
  static MasterTickServer mts;
  MasterTickServer& obtain_master_tick_server() { return mts; }
}

int main(int argc, char* argv[]) {
  return SSA::ss_app.run(argc, argv);
}
