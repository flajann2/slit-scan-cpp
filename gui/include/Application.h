// application.h of Slit Scan
#pragma once

#include <giomm/resource.h>
#include <gio/gio.h>

extern "C" GResource *gresource_get_resource (void);

using namespace std;

class SlitScanApp : public Gtk::Application {
public:
  SlitScanApp(std::string appname = "de.atomlogik.slit-scan")
    : Gtk::Application(appname) {
  }

  static SlitScanApp& get_ss_app() { return ss_app; }

  friend int main(int argc, char* argv[]);
  
protected:
  
  // Override the startup method to create the window and ball
  virtual void on_startup() override {
    Gtk::Application::on_startup();
    
    main_builder = Gui::obtain_builder();
    main_window = main_builder->get_object<Gtk::Window>(ss::main_window); 
    main_window->show();
    add_window(*main_window);

    // functionality
    gel_selection = Gui::GelSelection::create();
  }

private:
  shared_ptr<Gtk::Builder> main_builder;
  shared_ptr<Gtk::Window> main_window;

  unique_ptr<Gui::GelSelection> gel_selection;

  static SlitScanApp ss_app;
};

using SSA = class SlitScanApp;
