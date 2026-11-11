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
      double sweep_rate; // of the slit across a still gel, in gel widths per second
    };

    // what the settings are when the application starts
    static constexpr Settings defaults() { return Settings{3, 0, 10.0, 0.05}; }

    MainSettings() {
      main_settings = defaults(); // TODO: semi-temporary. Load from yaml

      auto builder = obtain_builder();

      // widgets
      main_settings_dialog = builder->get_object<Gtk::Dialog>(ss::main_settings);
      float_over_main_window(*main_settings_dialog);

      main_width_e = builder->get_object<Gtk::Entry>(ss::main_settings_p1_width_e);
      main_spacing_e = builder->get_object<Gtk::Entry>(ss::main_settings_p1_spacing_e);
      main_factor_e = builder->get_object<Gtk::Entry>(ss::main_settings_p1_factor_e);
      main_sweep_e = builder->get_object<Gtk::Entry>(ss::main_settings_p1_sweep_e);

      // Signals and slots
      main_settings_dialog->signal_close_request().connect(sigc::mem_fun(*this, &MainSettings::on_close), false);
      main_width_e->signal_insert_text().connect(sigc::mem_fun(*this
                                                               , &MainSettings::on_insert_text)
                                                 , true);

      // field mappings
      main_width_editor = make_unique<Gui::EntryEditor<int>>(main_settings.slit_width, main_width_e);
      main_spacing_editor = make_unique<Gui::EntryEditor<int>>(main_settings.slit_spacing, main_spacing_e);
      main_factor_editor = make_unique<Gui::EntryEditor<double>>(main_settings.perspective_factor, main_factor_e);
      main_sweep_editor = make_unique<Gui::EntryEditor<double>>(main_settings.sweep_rate, main_sweep_e);
    }

    // Back to the settings the application starts with.
    void reset() { set(defaults()); }

    // These are the settings now, in the dialog as well.
    void set(const Settings& settings) {
      main_settings = settings;
      show_settings();
    }

  protected:
    shared_ptr<Gtk::Dialog> main_settings_dialog;

    // every box shows what its setting really is
    void show_settings() {
      main_width_editor->refresh();
      main_spacing_editor->refresh();
      main_factor_editor->refresh();
      main_sweep_editor->refresh();
    }

    bool on_close() {
      cout << "MainSettings close event" << endl;
      show_settings(); // anything left half typed is dropped
      main_settings_dialog->hide();
      return true;
    }

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
    shared_ptr<Gtk::Entry> main_sweep_e;
    unique_ptr<Gui::EntryEditor<double>> main_sweep_editor;
    
  public:
    //friend const MainSettings::Settings& obtain_main_settings(); 
    static Settings main_settings;
    static unique_ptr<MainSettings> create() { return make_unique<MainSettings>(); }
    
    void show () { main_settings_dialog->show(); }
    void hide () { main_settings_dialog->hide(); }
  };

  const MainSettings::Settings& obtain_main_settings() {
    return MainSettings::main_settings;
  }
}
