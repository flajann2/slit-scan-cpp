// GelConfig.h of SlitScan -- configure individual gels
// for slitscanning.
#pragma once

using namespace std;

namespace Gui {
  class GelConfig : public Composition {
  public:
    GelConfig() {
      auto builder = obtain_builder();

      // acquire widgets
      gel_config = builder->get_object<Gtk::Dialog>(ss::gel_config);

      
    }

    static unique_ptr<GelConfig> create() { return make_unique<GelConfig>(); }

    void show() {
      gel_config->show();
    }

    void hide() {
    }
    
  protected:
  private:
    shared_ptr<Gtk::Dialog> gel_config;

  };
}
