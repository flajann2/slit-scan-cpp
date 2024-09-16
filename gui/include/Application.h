// application.h of Slit Scan
#pragma once

#include <giomm/resource.h>
#include <gio/gio.h>

extern "C" GResource *gresource_get_resource (void);

using namespace std;

using spBuilder = shared_ptr<Gtk::Builder>;

class SlitScanApp : public Gtk::Application {
public:
  SlitScanApp(std::string appname = "de.atomlogik.slit-scan")
    : Gtk::Application(appname) {
  }

  GResource& get_resource() { return *resource; }
  spBuilder get_builder() { return main_builder; }
  static SlitScanApp& get_ss_app() { return ss_app; }

  friend int main(int argc, char* argv[]);
  
protected:
  
  // Override the startup method to create the window and ball
  virtual void on_startup() override {
    Gtk::Application::on_startup();
    
    resource = gresource_get_resource(); // this loads and "registers" the resouce by way of PFM!
    main_builder = Gtk::Builder::create_from_resource(ss::resource + "slitscan_main.ui");
    main_window = main_builder->get_object<Gtk::Window>(ss::main_window); 
    main_window->show();
    add_window(*main_window);
  }

private:
  GResource* resource;
  spBuilder main_builder;
  shared_ptr<Gtk::Window> main_window;
  static SlitScanApp ss_app;
};

using SSA = class SlitScanApp;
