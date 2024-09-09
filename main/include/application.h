// application.h of Slit Scan
#pragma once

#include <gui.h>

using namespace std;

class SlitScanApp : public Gtk::Application {
public:
  SlitScanApp(std::string appname = "de.atomlogik.slit-scan")
    : Gtk::Application(appname) {
  }

protected:
  
  // Override the startup method to create the window and ball
  virtual void on_startup() override {
    Gtk::Application::on_startup();
    main_builder = Gtk::Builder::create_from_file("slitscan_main.ui");
  }

private:
  shared_ptr<Gtk::Builder> main_builder;
};
