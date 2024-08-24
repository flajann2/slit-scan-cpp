// AppWindow of SlitScan -- main or first window upon launch
#pragma once

class AppWindow : public Gui::Window {
public:
  AppWindow() {
    set_title("Slit Scan");
    show();
  }
};
