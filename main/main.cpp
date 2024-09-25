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
  shared_ptr<Gtk::Builder> obtain_builder() {
    if (!resource)
      resource = gresource_get_resource(); // this loads and "registers" the resouce by way of PFM!

    if (!app_builder) {
      app_builder = Gtk::Builder::create_from_resource(ss::resource + "slitscan_main.ui");
      app_builder->add_from_resource(ss::resource + "file_gel_select.ui");
    }
    return app_builder;
  }

  MainSettings::Settings MainSettings::main_settings;
}

int main(int argc, char* argv[]) {
  return SSA::ss_app.run(argc, argv);
}
