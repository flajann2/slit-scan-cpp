// GelSelection.h of SlitScan -- allow selection and management of gels
// for slitscanning.
#pragma once

#include <gels.h>

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

      toggle_play   = builder->get_object<Gtk::ToggleButton>(ss::main_toggle_play);
      video_setup   = builder->get_object<Gtk::Button>(ss::main_video_setup);
      toggle_record = builder->get_object<Gtk::ToggleButton>(ss::main_toggle_record);
      
      main_settings_dialog = MainSettings::create();
      video_config_dialog = VideoConfig::create();

      // the slit scanner, and the canvas it paints to
      slit_scan = make_unique<Gel::SlitScan>();
      slit_scan->signal_next_frame().connect(sigc::mem_fun(*this, &GelSelection::on_scan_frame));
      canvas->set_draw_func(sigc::mem_fun(*this, &GelSelection::on_draw));
      ticker.signal_tick().connect(sigc::mem_fun(*this, &GelSelection::on_tick));
      
      // signal-slots configuation
      main_load_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_load));
      main_save_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_save));
      main_save_as_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_save_as));
      main_new_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_new));
      main_settings_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_settings));

      gel_left_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_left_gel));
      gel_right_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_right_gel));
      gel_chooser->signal_response().connect(sigc::mem_fun(*this, &GelSelection::on_chooser_response));

      toggle_play->  signal_toggled().connect(sigc::mem_fun(*this, &GelSelection::on_play_toggled));
      video_setup->  signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_video_setup));
      toggle_record->signal_toggled().connect(sigc::mem_fun(*this, &GelSelection::on_record_toggled));

      // selecting a gel in a list makes it the one being scanned
      gel_left_listbox->signal_selected_rows_changed().connect(sigc::mem_fun(*this, &GelSelection::refresh_active_gels));
      gel_right_listbox->signal_selected_rows_changed().connect(sigc::mem_fun(*this, &GelSelection::refresh_active_gels));

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

    // What the scanner should be doing, from the global settings
    // and the video configuration.
    Gel::SlitScan::Params scan_params() {
      const auto& ms = obtain_main_settings();
      const auto& vc = VideoConfig::obtain_video_configiguration();

      Gel::SlitScan::Params params;
      params.width  = vc.res_width;
      params.height = vc.res_height;
      params.slit_width   = ms.slit_width;
      params.slit_spacing = ms.slit_spacing;
      params.perspective_factor = ms.perspective_factor;
      return params;
    }

    void on_play_toggled() {
      playing = toggle_play->get_active();

      if (playing) {
        const auto& vc = VideoConfig::obtain_video_configiguration();
        const double fps = (vc.frames_per_sec > 0.0) ? vc.frames_per_sec : 30.0;

        slit_scan->configure(scan_params());
        slit_scan->resume();
        refresh_active_gels();
        ticker.set_interval(Gel::micros(static_cast<long long>(1000000.0 / fps)));
        ticker.start();
      } else {
        ticker.stop();
        refresh_active_gels();
      }
    }

    // every tick, while playing. Picks up changes to the settings
    // as they are made; configure() does nothing if nothing changed.
    void on_tick(Gel::time_stamp) {
      if (playing)
        slit_scan->configure(scan_params());
    }

    // The gel being scanned on a side: the first selected one in
    // that side's list, or failing that the first one in the list.
    shared_ptr<Gui::GelAsset> active_gel(GelAsset::Placement place) {
      auto& listbox = (place == GelAsset::Placement::left) ? gel_left_listbox
                                                           : gel_right_listbox;
      shared_ptr<Gui::GelAsset> first;
      for (auto& ga : assets) {
        if (ga->get_placement() != place)
          continue;
        if (!first)
          first = ga;
        for (auto* row : listbox->get_selected_rows()) {
          if (row && row->get_child() == ga.get())
            return ga;
        }
      }
      return first;
    }

    // Only the active gels broadcast frames, and only while playing.
    void refresh_active_gels() {
      auto active_left  = active_gel(GelAsset::Placement::left);
      auto active_right = active_gel(GelAsset::Placement::right);

      for (auto& ga : assets)
        ga->source().set_active(playing && (ga == active_left || ga == active_right));
    }

    void on_video_setup() {
      video_config_dialog->show();
    }

    void on_record_toggled() { }
    
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
            ga->source().signal_next_frame().connect(sigc::mem_fun(*this, &GelSelection::on_next_left_frame));
            break;

          case GelAsset::Placement::right:
            gel_right_listbox->append(*ga);
            ga->source().signal_next_frame().connect(sigc::mem_fun(*this, &GelSelection::on_next_right_frame));
            break;
            
          default:
            break;
          }
          chosen_files.push_back(file->get_path());
          assets.push_back(ga);
        }
        refresh_active_gels();
      }
    }

    void on_next_left_frame(const cv::Mat& frame, Gel::time_stamp ts) {
      slit_scan->push_frame(Gel::SlitScan::Side::Left, frame, ts);
    }
    
    void on_next_right_frame(const cv::Mat& frame, Gel::time_stamp ts) {
      slit_scan->push_frame(Gel::SlitScan::Side::Right, frame, ts);
    }

    // a finished frame from the scanner
    void on_scan_frame(const cv::Mat& frame, Gel::time_stamp) {
      scan_frame = frame;
      canvas->queue_draw();
    }

    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr
                 , int width
                 , int height) {
      Gel::frame_to_canvas(scan_frame
                           , cr
                           , Gel::F2CC{width, height, true, true});
    }
    
  private:
    GelAsset::Placement current_place = GelAsset::Placement::undefined; // what the user has chosen

    // slit scanning and playback
    unique_ptr<Gel::SlitScan> slit_scan;
    Gel::IMasterTickServer ticker;
    bool playing = false;
    cv::Mat scan_frame; // the latest finished frame, for the canvas

    unique_ptr<Gui::MainSettings> main_settings_dialog;
    unique_ptr<Gui::VideoConfig> video_config_dialog;
    
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

    shared_ptr<Gtk::ToggleButton> toggle_play;
    shared_ptr<Gtk::Button> video_setup;
    shared_ptr<Gtk::ToggleButton> toggle_record;

    Glib::RefPtr<Gtk::FileFilter> filter;
    shared_ptr<Gtk::FileChooserNative> gel_chooser;
    list<string> chosen_files;
    list<shared_ptr<Gui::GelAsset>> assets; // we filter on Placement
 };
}
