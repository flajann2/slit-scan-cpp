// application.h of Slit Scan
#pragma once

#include <giomm/resource.h>
#include <gio/gio.h>

using namespace std;

class SlitScanApp : public Gtk::Application {
public:
  // NON_UNIQUE: every launch is an application of its own.
  //
  // Without it there can only be one SlitScan per login session:
  // a second one finds the first over D-Bus, asks it to come to
  // the front, and exits at once with no window and no message.
  // That is a fine thing in a mail program. Here it means that one
  // copy left running out of sight, or stuck, silently stops every
  // later launch from starting.
  SlitScanApp(std::string appname = "de.atomlogik.slit-scan")
    : Gtk::Application(appname, Gio::Application::Flags::NON_UNIQUE) {
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
