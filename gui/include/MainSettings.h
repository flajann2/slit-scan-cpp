// MainSettings.h of SlitScan -- global settings
// for slitscanning.
#pragma once

namespace Gui {
  using namespace std;
  
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
      main_settings_dialog = builder->get_object<Gtk::Dialog>(ss::main_settings);

      // widgets
      main_width_e = builder->get_object<Gtk::Entry>(ss::main_settings_p1_width_e);
      main_spacing_e = builder->get_object<Gtk::Entry>(ss::main_settings_p1_spacing_e);
      main_factor_e = builder->get_object<Gtk::Entry>(ss::main_settings_p1_factor_e);
      
      main_width_e->signal_insert_text().connect(sigc::mem_fun(*this
                                                               , &MainSettings::on_insert_text)
                                                 , true);

      // field mappings
      main_width_editor = make_unique<Gui::EntryEditor<int>>(main_settings.slit_width, main_width_e);
      main_spacing_editor = make_unique<Gui::EntryEditor<int>>(main_settings.slit_spacing, main_spacing_e);
      main_factor_editor = make_unique<Gui::EntryEditor<double>>(main_settings.perspective_factor, main_factor_e);
    }
    

  protected:
    shared_ptr<Gtk::Dialog> main_settings_dialog;

    void on_insert_text(const Glib::ustring& text, int* position) {
      cout << "on_insert_text: " << text << "at " << * position << endl;
      std::string filtered_text;
      for (char c : text) {
        if (std::isdigit(c) || (c == '-' && *position == 0)) {
          filtered_text += c;
        }
      }
      
      if (filtered_text != text.raw()) {
        main_width_e->insert_text(filtered_text
                                  , filtered_text.length()
                                  , *position);
        main_width_e->signal_insert_text().emission_stop();
      }
    }
    
  private:
    shared_ptr<Gtk::Entry> main_width_e;
    unique_ptr<Gui::EntryEditor<int>> main_width_editor;
    shared_ptr<Gtk::Entry> main_spacing_e;
    unique_ptr<Gui::EntryEditor<int>> main_spacing_editor;
    shared_ptr<Gtk::Entry> main_factor_e;
    unique_ptr<Gui::EntryEditor<double>> main_factor_editor;
    
  public:
    //friend const MainSettings::Settings& obtain_main_settings(); 
    static Settings main_settings;
    static unique_ptr<MainSettings> create() { return make_unique<MainSettings>(); }
    
    void show () {
      main_settings_dialog->show();
    }
  };

  const MainSettings::Settings& obtain_main_settings() {
    return MainSettings::main_settings;
  }
}
