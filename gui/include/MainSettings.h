// MainSettings.h of SlitScan -- global settings
// for slitscanning: the Settings dialog.
//
// What it sets is a Gui::Settings (see Settings.h, which also has
// the file the user's settings are kept in). The dialog knows
// nothing of files. It says when the user has changed something,
// by signal_edited(), and GelSelection sees to the rest.
#pragma once

namespace Gui {
  using namespace std;
  
  class MainSettings : public Composition {
  public:
    using Settings = Gui::Settings;

    // what the settings are with no file to say otherwise
    static Settings defaults() { return Settings::defaults(); }

    MainSettings() {
      main_settings = defaults(); // until GelSelection has read the user's file

      auto builder = obtain_builder();

      // widgets
      main_settings_dialog = builder->get_object<Gtk::Dialog>(ss::main_settings);
      float_over_main_window(*main_settings_dialog);

      main_width_e = builder->get_object<Gtk::Entry>(ss::main_settings_p1_width_e);
      main_spacing_e = builder->get_object<Gtk::Entry>(ss::main_settings_p1_spacing_e);
      main_factor_e = builder->get_object<Gtk::Entry>(ss::main_settings_p1_factor_e);
      main_sweep_e = builder->get_object<Gtk::Entry>(ss::main_settings_p1_sweep_e);
      video_folder_e = builder->get_object<Gtk::Entry>(ss::main_settings_p2_folder_e);
      video_folder_button = builder->get_object<Gtk::Button>(ss::b_main_settings_p2_folder);

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

      // the folder for recordings: typed in, or chosen with the button
      if (video_folder_e) {
        // empty stands for the default, and the box says which that is
        video_folder_e->set_placeholder_text(Paths::default_video_folder());
        video_folder_e->signal_changed().connect(sigc::mem_fun(*this, &MainSettings::on_video_folder_typed));
      }
      if (video_folder_button)
        video_folder_button->signal_clicked().connect(sigc::mem_fun(*this, &MainSettings::on_choose_video_folder));
      show_settings();
    }

    // Back to what the settings are with no file to say otherwise.
    void reset() { set(defaults()); }

    // These are the settings now, in the dialog as well.
    void set(const Settings& settings) {
      main_settings = settings;
      show_settings();
    }

    // Emitted when the dialog is closed and the user has changed
    // something in it: with the settings as they were when it was
    // opened, and as they are now.
    sigc::signal<void(const Settings&, const Settings&)>& signal_edited() { return sig_edited; }

  protected:
    shared_ptr<Gtk::Dialog> main_settings_dialog;

    // every box shows what its setting really is
    void show_settings() {
      main_width_editor->refresh();
      main_spacing_editor->refresh();
      main_factor_editor->refresh();
      main_sweep_editor->refresh();
      if (video_folder_e)
        video_folder_e->set_text(main_settings.video_folder);
    }

    bool on_close() {
      cout << "MainSettings close event" << endl;
      show_settings(); // anything left half typed is dropped
      main_settings_dialog->hide();
      if (main_settings != as_opened) {
        const Settings before = as_opened;
        as_opened = main_settings; // should it be closed twice
        sig_edited.emit(before, main_settings);
      }
      return true;
    }

    // What is typed into the folder box is the setting, as it
    // stands; "~" may be used for the home folder. The folder need
    // not exist yet: it is made when a recording is to go into it.
    void on_video_folder_typed() {
      main_settings.video_folder = Paths::expanded(video_folder_e->get_text());
    }

    // The button beside the box: the system's dialog for choosing
    // a folder, which opens at the folder the box has now.
    void on_choose_video_folder() {
      folder_chooser = Gtk::FileChooserNative::create("Folder for recorded videos"
                                                      , *main_settings_dialog
                                                      , Gtk::FileChooser::Action::SELECT_FOLDER
                                                      , "Choose", "Cancel");
      folder_chooser->set_modal(true);

      std::string at = main_settings.video_folder;
      if (at.empty() || !Glib::file_test(at, Glib::FileTest::IS_DIR))
        at = Paths::default_video_folder();
      try {
        folder_chooser->set_current_folder(Gio::File::create_for_path(at));
      } catch (const Glib::Error& e) {
        cout << "Could not open the folder chooser in " << at << ": " << e.what() << endl;
      }

      folder_chooser->signal_response().connect([this](int response) {
        if (response == GTK_RESPONSE_ACCEPT) {
          if (auto folder = folder_chooser->get_file(); folder && !folder->get_path().empty())
            video_folder_e->set_text(folder->get_path()); // and so the setting; see on_video_folder_typed()
        }
        folder_chooser.reset();
      });
      folder_chooser->show();
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
    shared_ptr<Gtk::Entry> video_folder_e;
    shared_ptr<Gtk::Button> video_folder_button;
    Glib::RefPtr<Gtk::FileChooserNative> folder_chooser; // while the user is choosing

    Settings as_opened; // the settings when the dialog was last opened
    sigc::signal<void(const Settings&, const Settings&)> sig_edited;
    
  public:
    //friend const MainSettings::Settings& obtain_main_settings(); 
    static Settings main_settings;
    static unique_ptr<MainSettings> create() { return make_unique<MainSettings>(); }
    
    void show () {
      as_opened = main_settings;
      main_settings_dialog->show();
    }
    void hide () { main_settings_dialog->hide(); }
  };

  const MainSettings::Settings& obtain_main_settings() {
    return MainSettings::main_settings;
  }
}
