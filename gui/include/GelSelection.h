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
      cam_left_button  = builder->get_object<Gtk::Button>(ss::main_gel_cam_left);
      cam_right_button = builder->get_object<Gtk::Button>(ss::main_gel_cam_right);
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
      slit_scan->signal_recording_finished().connect(sigc::mem_fun(*this, &GelSelection::on_recording_finished));
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
      cam_left_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_left_camera));
      cam_right_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_right_camera));
      gel_chooser->signal_response().connect(sigc::mem_fun(*this, &GelSelection::on_chooser_response));

      toggle_play->  signal_toggled().connect(sigc::mem_fun(*this, &GelSelection::on_play_toggled));
      video_setup->  signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_video_setup));
      toggle_record->signal_toggled().connect(sigc::mem_fun(*this, &GelSelection::on_record_toggled));

      // selecting a gel in a list makes it the one being scanned
      gel_left_listbox->signal_selected_rows_changed().connect(sigc::mem_fun(*this, &GelSelection::refresh_active_gels));
      gel_right_listbox->signal_selected_rows_changed().connect(sigc::mem_fun(*this, &GelSelection::refresh_active_gels));

      // Gel Chooser filters: images and videos
      filter = Gtk::FileFilter::create();
      filter->set_name("Images and videos");
      filter->add_mime_type("image/jpeg");
      filter->add_mime_type("image/png");
      filter->add_mime_type("image/gif");
      filter->add_mime_type("image/webp");
      filter->add_mime_type("video/*");
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

    // The camera as a gel. It is the default camera, and one camera
    // can only be opened once, so one camera gel is all there is.
    void on_left_camera()  { add_camera(GelAsset::Placement::left); }
    void on_right_camera() { add_camera(GelAsset::Placement::right); }

    void add_camera(GelAsset::Placement place) {
      for (auto& ga : assets) {
        if (ga->get_kind() == GelAsset::Kind::camera) {
          cout << "The camera is already a gel." << endl;
          return;
        }
      }
      add_asset(GelAsset::create_camera_asset(0, place));
      refresh_active_gels();
    }

    // Puts a gel into its list and hooks its frames up to the scanner.
    void add_asset(shared_ptr<Gui::GelAsset> ga) {
      if (!ga)
        return;

      switch(ga->get_placement()) {
      case GelAsset::Placement::left:
        gel_left_listbox->append(*ga);
        ga->source().signal_next_frame().connect(sigc::mem_fun(*this, &GelSelection::on_next_left_frame));
        break;

      case GelAsset::Placement::right:
        gel_right_listbox->append(*ga);
        ga->source().signal_next_frame().connect(sigc::mem_fun(*this, &GelSelection::on_next_right_frame));
        break;

      default:
        return;
      }
      assets.push_back(ga);
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
      params.sweep_rate = ms.sweep_rate;

      // the slit sweeps across still images, and holds still for
      // videos and cameras, which move by themselves.
      auto active_left  = active_gel(GelAsset::Placement::left);
      auto active_right = active_gel(GelAsset::Placement::right);
      params.sweep_left  = !active_left  || !active_left->source().is_moving();
      params.sweep_right = !active_right || !active_right->source().is_moving();
      return params;
    }

    // Where a recording goes: the path from the video configuration
    // if there is one, and otherwise a new file, named for the
    // time, in the user's Videos folder.
    std::string recording_pathname(const VideoConfig::Configuration& vc) {
      if (!vc.pathnname.empty())
        return vc.pathnname;

      std::string dir = Glib::get_user_special_dir(Glib::UserDirectory::VIDEOS);
      if (dir.empty() || !Glib::file_test(dir, Glib::FileTest::IS_DIR))
        dir = Glib::get_home_dir();

      const auto stamp = Glib::DateTime::create_now_local().format("%Y%m%d-%H%M%S");
      return Glib::build_filename(dir, "slitscan-" + stamp.raw() + "." + vc.format);
    }

    // PLAY is a toggle: the first click starts the scan and the
    // second stops it.
    void on_play_toggled() {
      playing = toggle_play->get_active();
      cout << (playing ? "PLAY on" : "PLAY off") << endl;

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
        slit_scan->discard_pending(); // stop now, not after the backlog
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

    // REC records what the scanner produces, so there is only
    // something to record while PLAY is on. The first click starts
    // a new video file and the second ends it.
    //
    // Nothing here waits for the video encoder. Ending a recording
    // only says "no more frames"; the file is closed in the
    // background and on_recording_finished() hears of it. That way
    // the buttons keep answering whatever the encoder is up to.
    void on_record_toggled() {
      if (!toggle_record->get_active()) {
        if (slit_scan->stop_recording()) {
          closing_recordings.insert(recording_to);
          report_recording("Closing " + recording_to + " ...");

          // if the encoder has still not let go of the file after
          // a while, say so instead of leaving the user guessing.
          const std::string pathname = recording_to;
          Glib::signal_timeout().connect_once(
            sigc::track_obj([this, pathname] { on_closing_overdue(pathname); }, *this)
            , 5000);
        }
        return;
      }

      const auto& vc = VideoConfig::obtain_video_configiguration();
      const auto params = scan_params();
      const double fps = (vc.frames_per_sec > 0.0) ? vc.frames_per_sec : 30.0;
      recording_to = recording_pathname(vc);

      Gel::Recorder::Request request;
      request.pathname = recording_to;
      request.format = vc.format;
      request.codec = vc.codec;
      request.fps = fps;
      request.size = cv::Size(params.width, params.height);

      slit_scan->configure(params);
      if (slit_scan->start_recording(request)) {
        recording_how = slit_scan->recording_description();
        // REC with PLAY off is armed, but has nothing to record yet
        report_recording("Recording to " + recording_to
                         + " as " + recording_how
                         + (playing ? "" : " -- nothing is recorded until PLAY is on"));
      } else {
        report_recording("Could not record to " + recording_to);
        toggle_record->set_active(false); // comes back through here, harmlessly
      }
    }

    // the recorder has closed a file
    void on_recording_finished(const Gel::Recorder::Result& result) {
      closing_recordings.erase(result.pathname);
      if (result.frames > 0 && result.ok)
        report_recording(std::format("Recorded {} frames to {} as {}"
                                     , result.frames, result.pathname, result.how));
      else if (result.frames > 0)
        report_recording(std::format("The encoder ({}) failed; {} may be incomplete. "
                                     "The terminal has the encoder's own words."
                                     , result.how, result.pathname));
      else if (!result.ok)
        report_recording(std::format("The encoder ({}) failed and nothing was recorded. "
                                     "The terminal has the encoder's own words."
                                     , result.how));
      else
        report_recording("Nothing was recorded, so no file was kept. "
                         "REC records while PLAY is on.");
    }

    void on_closing_overdue(const std::string& pathname) {
      if (closing_recordings.count(pathname))
        report_recording("The encoder (" + recording_how
                         + ") is still busy closing " + pathname + ".");
    }

    // Says what the recording is doing: on the terminal, and as the
    // tooltip of REC. The tooltip is set a moment later, from the
    // main loop, to keep the click handler down to the bare click.
    void report_recording(const std::string& message) {
      cout << message << endl;
      Glib::signal_idle().connect_once(
        sigc::track_obj([this, message] { toggle_record->set_tooltip_text(message); }, *this));
    }
    
    void on_chooser_response(int r_id) {
      chosen_files.clear();
      if (r_id == GTK_RESPONSE_ACCEPT) {
        // Get the selected folder path
        auto filelist = gel_chooser->get_files2();
        for (auto file : filelist) {
          // nothing comes back if the file is neither an image nor a video
          auto ga = GelAsset::create_gel_asset(file->get_path(), current_place);
          if (!ga)
            continue;

          add_asset(ga);
          chosen_files.push_back(file->get_path());
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
    std::string recording_to; // the file being recorded, or last recorded
    std::string recording_how; // the codec and encoder writing it
    std::set<std::string> closing_recordings; // stopped, not yet closed

    unique_ptr<Gui::MainSettings> main_settings_dialog;
    unique_ptr<Gui::VideoConfig> video_config_dialog;
    
    shared_ptr<Gtk::Button> main_load_button;
    shared_ptr<Gtk::Button> main_save_button;
    shared_ptr<Gtk::Button> main_save_as_button;
    shared_ptr<Gtk::Button> main_new_button;
    shared_ptr<Gtk::Button> main_settings_button;
    shared_ptr<Gtk::Button> gel_left_button;
    shared_ptr<Gtk::Button> gel_right_button;
    shared_ptr<Gtk::Button> cam_left_button;
    shared_ptr<Gtk::Button> cam_right_button;
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
