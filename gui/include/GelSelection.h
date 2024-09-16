// GelSelection.h of SlitScan -- allow selection and management of gels
// for slitscanning.
#pragma once

using namespace std;

namespace Gui {
  class GelSlection : public Gui::Composition {
  public:
    GelSlection() {
      auto builder = SlitScanApp::get_ss_app().get_builder();
    }
    
  private:
    shared_ptr<Gtk::Button> gel_left_button;
    shared_ptr<Gtk::Button> gel_right_button;
    shared_ptr<Gtk::ListBox> gel_left_listbox;
    shared_ptr<Gtk::ListBox> gel_right_listbox;  
  };
}
