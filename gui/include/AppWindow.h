// AppWindow of SlitScan -- main or first window upon launch
#pragma once

namespace Gui {
  class AppWindow : public Gui::Window {
  public:
    AppWindow() {
      set_title("Slit Scan");
      show();
    }
  };
}
