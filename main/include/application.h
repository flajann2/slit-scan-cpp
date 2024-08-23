// application.h of Slit Scan
#pragma once

#include <gui.h>

class SlitScanApp : public Gtk::Application {
public:
  SlitScanApp(std::string appname = "de.atomlogik.escáner")
    : Gtk::Application(appname) {
  }

protected:
  
  // Override the startup method to create the window and ball
  void on_activate() override {
    window.set_title("Escáner de Mercado");
    window.show();
    this->add_window(window);
  }
};
