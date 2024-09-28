// MainSettings.h of SlitScan -- global settings
// for slitscanning.
#pragma once

namespace Gui {
  class MainSettings : public Composition {
  public:
    struct Settings {
      int slit_width; // Width of each slit
      int slit_spacing; // Space between slits
      double perspective_factor;
    };

    MainSettings() {
      main_settings = Settings(3, 0, 10.0); // TODO: semi-temporary. Load from yaml
      auto builder = obtain_builder();
    }
    
  private:
  public:
    //friend const MainSettings::Settings& obtain_main_settings(); 
    static Settings main_settings;
  };

  const MainSettings::Settings& obtain_main_settings() {
    return MainSettings::main_settings;
  }
}
