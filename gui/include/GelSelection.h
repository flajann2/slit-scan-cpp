// GelSelection.h of SlitScan -- allow selection and management of gels
// for slitscanning.
#pragma once

using namespace std;

namespace Gui {
  class GelSlection : public Gui::Composition {
  public:
    GelSlection() {
      auto builder = obtain_builder();

      // acquire widget elements
      gel_left_button  = builder->get_object<Gtk::Button>(ss::main_gel_get_left);
      gel_right_button = builder->get_object<Gtk::Button>(ss::main_gel_get_right); 
      gel_left_listbox  = builder->get_object<Gtk::ListBox>(ss::main_gel_list_left);
      gel_right_listbox = builder->get_object<Gtk::ListBox>(ss::main_gel_list_right);

      // signal-slots configuation
      gel_left_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSlection::on_left_gel));
      cout << "wired up" << endl;
    }

    static unique_ptr<GelSlection> create() { return make_unique<GelSlection>(); }

  protected:
    void on_left_gel() {
      cout << "left gel clicked." << endl;
    }
    
  private:
    shared_ptr<Gtk::Button> gel_left_button;
    shared_ptr<Gtk::Button> gel_right_button;
    shared_ptr<Gtk::ListBox> gel_left_listbox;
    shared_ptr<Gtk::ListBox> gel_right_listbox;  
  };
}
