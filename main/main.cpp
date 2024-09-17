// SlitScan main

#include <main.h>
#include <Builder.h>
//#include <hekate/hekate>

using namespace std;

SSA SSA::ss_app;

namespace Gui {
  GResource* resource = nullptr;
  spBuilder app_builder;
  spBuilder obtain_builder() {
    if (!resource)
      resource = gresource_get_resource(); // this loads and "registers" the resouce by way of PFM!

    if (!app_builder)
      app_builder = Gtk::Builder::create_from_resource(ss::resource + "slitscan_main.ui");

    return app_builder;
  }
  
}

int main(int argc, char* argv[]) {
  return SSA::ss_app.run(argc, argv);
}
