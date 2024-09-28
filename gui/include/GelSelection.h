// GelSelection.h of SlitScan -- allow selection and management of gels
// for slitscanning.
#pragma once

using namespace std;

namespace Gui {
  class GelSelection : public Gui::Composition {
  public:
    GelSelection() {
      auto builder = obtain_builder();

      // acquire widget elements
      main_load_button     = builder->get_object<Gtk::Button>(ss::b_main_load);
      main_save_button     = builder->get_object<Gtk::Button>(ss::b_main_save);
      main_save_as_button  = builder->get_object<Gtk::Button>(ss::b_main_save_as);
      main_new_button      = builder->get_object<Gtk::Button>(ss::b_main_new);
      main_settings_button = builder->get_object<Gtk::Button>(ss::b_main_settings);

      gel_left_button  = builder->get_object<Gtk::Button>(ss::main_gel_get_left);
      gel_right_button = builder->get_object<Gtk::Button>(ss::main_gel_get_right); 
      gel_left_listbox  = builder->get_object<Gtk::ListBox>(ss::main_gel_list_left);
      gel_right_listbox = builder->get_object<Gtk::ListBox>(ss::main_gel_list_right);
      gel_chooser = builder->get_object<Gtk::FileChooserNative>(ss::file_gel_chooser);
      canvas = builder->get_object<Gtk::DrawingArea>(ss::main_canvas);

      main_settings_dialog = MainSettings::create();

      // signal-slots configuation
      main_load_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_load));
      main_save_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_save));
      main_save_as_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_save_as));
      main_new_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_new));
      main_settings_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_settings));

      gel_left_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_left_gel));
      gel_right_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_right_gel));
      gel_chooser->signal_response().connect(sigc::mem_fun(*this, &GelSelection::on_chooser_response));

      // Gel Chooser filters
      filter = Gtk::FileFilter::create();
      filter->add_mime_type("image/jpeg");
      filter->add_mime_type("image/png");
      filter->add_mime_type("image/gif");
      filter->add_mime_type("image/webp");
      gel_chooser->add_filter(filter);
    }

    static unique_ptr<GelSelection> create() { return make_unique<GelSelection>(); }

  protected:
    void on_load() {
      cout << "Load clicked." << endl;
    }

    void on_save() {
      cout << "Save clicked." << endl;
    }

    void on_save_as() {
      cout << "Save as clicked." << endl;
    }

    void on_new() {
      cout << "New clicked." << endl;
    }

    void on_settings() {
      cout << "Settings clicked." << endl;
      main_settings_dialog->show();
    }
    
    void on_left_gel() {
      cout << "left gel clicked." << endl;
      current_place = GelAsset::Placement::left;
      gel_chooser->show();
    }
    
    void on_right_gel() {
      cout << "right gel clicked." << endl;
      current_place = GelAsset::Placement::right;
      gel_chooser->show();
    }

    void on_chooser_response(int r_id) {
      chosen_files.clear();
      if (r_id == GTK_RESPONSE_ACCEPT) {
        // Get the selected folder path
        auto filelist = gel_chooser->get_files2();
        for (auto file : filelist) {
          auto ga = GelAsset::create_gel_asset(file->get_path(), current_place);

          switch(current_place) {
          case GelAsset::Placement::left:
            gel_left_listbox->append(*ga);
            break;

          case GelAsset::Placement::right:
            gel_right_listbox->append(*ga);
            break;
            
          default:
            break;
          }
          chosen_files.push_back(file->get_path());
          assets.push_back(ga);
        }        
      }
    }
    
  private:
    GelAsset::Placement current_place; // what the user has chosen

    unique_ptr<Gui::MainSettings> main_settings_dialog;
    
    shared_ptr<Gtk::Button> main_load_button;
    shared_ptr<Gtk::Button> main_save_button;
    shared_ptr<Gtk::Button> main_save_as_button;
    shared_ptr<Gtk::Button> main_new_button;
    shared_ptr<Gtk::Button> main_settings_button;
    shared_ptr<Gtk::Button> gel_left_button;
    shared_ptr<Gtk::Button> gel_right_button;
    shared_ptr<Gtk::ListBox> gel_left_listbox;
    shared_ptr<Gtk::ListBox> gel_right_listbox;
    shared_ptr<Gtk::DrawingArea> canvas;

    Glib::RefPtr<Gtk::FileFilter> filter;
    shared_ptr<Gtk::FileChooserNative> gel_chooser;
    list<string> chosen_files;
    list<shared_ptr<Gui::GelAsset>> assets; // we filter on Placement
 };
}
