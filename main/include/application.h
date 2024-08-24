// application.h of Slit Scan
#pragma once

#include <gui.h>

class SlitScanApp : public Gtk::Application {
public:
  SlitScanApp(std::string appname = "de.atomlogik.slit-scan")
    : Gtk::Application(appname) {
  }

protected:
  
  // Override the startup method to create the window and ball
  void on_activate() override {
    //this->add_window(window);
  }
};
