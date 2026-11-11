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
      restart_button = builder->get_object<Gtk::Button>(ss::main_restart);
      main_window    = builder->get_object<Gtk::Window>(ss::main_window);
      video_setup   = builder->get_object<Gtk::Button>(ss::main_video_setup);
      toggle_record = builder->get_object<Gtk::ToggleButton>(ss::main_toggle_record);
      
      main_settings_dialog = MainSettings::create();
      video_config_dialog = VideoConfig::create();

      // the slit scanner, and the canvas it paints to
      slit_scan = make_unique<Gel::SlitScan>();
      slit_scan->signal_next_frame().connect(sigc::mem_fun(*this, &GelSelection::on_scan_frame));
      slit_scan->signal_recording_finished().connect(sigc::mem_fun(*this, &GelSelection::on_recording_finished));
      slit_scan->signal_pass_finished().connect(sigc::mem_fun(*this, &GelSelection::on_pass_finished));
      canvas->set_draw_func(sigc::mem_fun(*this, &GelSelection::on_draw));
      ticker.signal_tick().connect(sigc::mem_fun(*this, &GelSelection::on_tick));
      
      // signal-slots configuation
      main_load_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_load));
      main_save_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_save));
      main_save_as_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_save_as));
      main_new_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_new));
      restart_button->signal_clicked().connect(sigc::mem_fun(*this, &GelSelection::on_restart));

      // A gel dropped anywhere in a side's column, other than on
      // another gel, goes to the end of that side's list. The
      // column and not the list itself, because a list is only as
      // tall as its gels: an empty one would be impossible to hit.
      for (auto place : {GelAsset::Placement::left, GelAsset::Placement::right}) {
        Gtk::Widget* column = listbox_of(place).get_parent();
        add_drop_target(column ? *column : listbox_of(place), place, nullptr);
      }
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
      gel_left_listbox->signal_selected_rows_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &GelSelection::on_gel_selected), GelAsset::Placement::left));
      gel_right_listbox->signal_selected_rows_changed().connect(
        sigc::bind(sigc::mem_fun(*this, &GelSelection::on_gel_selected), GelAsset::Placement::right));

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
    // ---- Load, Save and Save as. A setup is everything there is to
    // set: the settings, the video configuration, and the gels of
    // both sides with what is set in their dialogs. It is kept in a
    // YAML file; see Setup.h for what the file looks like.

    void on_load() {
      cout << "Load clicked." << endl;
      choose_setup_file(Gtk::FileChooser::Action::OPEN, "Load a setup", "Load"
                        , [this](const std::string& pathname) { load_setup(pathname); });
    }

    // Save goes to the file the setup was loaded from or last saved
    // to. A setup that has no file yet is asked for one.
    void on_save() {
      cout << "Save clicked." << endl;
      if (setup_pathname.empty())
        on_save_as();
      else
        save_setup(setup_pathname);
    }

    void on_save_as() {
      cout << "Save as clicked." << endl;
      choose_setup_file(Gtk::FileChooser::Action::SAVE, "Save the setup as", "Save"
                        , [this](std::string pathname) {
                            // a setup file is a YAML file, and is called one
                            if (!pathname.ends_with(".yaml") && !pathname.ends_with(".yml"))
                              pathname += ".yaml";
                            save_setup(pathname);
                          });
    }

    // Asks the user for a setup file, and hands its full pathname
    // to `then` if one was chosen.
    void choose_setup_file(Gtk::FileChooser::Action action
                           , const std::string& title
                           , const std::string& accept
                           , std::function<void(const std::string&)> then) {
      setup_chooser = Gtk::FileChooserNative::create(title, *main_window, action, accept, "Cancel");

      auto yaml = Gtk::FileFilter::create();
      yaml->set_name("Slit Scan setups (*.yaml)");
      yaml->add_pattern("*.yaml");
      yaml->add_pattern("*.yml");
      setup_chooser->add_filter(yaml);

      if (action == Gtk::FileChooser::Action::SAVE)
        setup_chooser->set_current_name(setup_pathname.empty()
                                        ? "slitscan.yaml"
                                        : Glib::path_get_basename(setup_pathname));

      setup_chooser->signal_response().connect([this, then](int response) {
        if (response == GTK_RESPONSE_ACCEPT) {
          if (auto file = setup_chooser->get_file())
            then(file->get_path());
        }
        setup_chooser.reset();
      });
      setup_chooser->show();
    }

    // The setup as it stands, gathered from the dialogs and the lists.
    Setup current_setup() {
      Setup setup;
      setup.settings = obtain_main_settings();
      setup.video = VideoConfig::obtain_video_configiguration();

      for (auto place : {GelAsset::Placement::left, GelAsset::Placement::right}) {
        auto& gels = (place == GelAsset::Placement::left) ? setup.left : setup.right;
        for (auto& ga : gels_of(place)) {
          Setup::GelEntry gel;
          gel.kind = ga->get_kind();
          if (gel.kind == GelAsset::Kind::camera)
            gel.device = ga->get_camera_index();
          else
            gel.path = Glib::canonicalize_filename(ga->get_pathname()); // the full pathname
          gel.rotate = ga->config().get_angle();
          gel.rate = ga->config().get_rate();
          gel.flip = ga->config().get_flip();
          gels.push_back(gel);
        }
      }
      return setup;
    }

    void save_setup(const std::string& pathname) {
      try {
        current_setup().save(pathname);
        set_setup_pathname(pathname);
        cout << "Saved the setup to " << pathname << endl;
      } catch (const std::exception& e) {
        tell_user("The setup was not saved.", e.what());
      }
    }

    // Everything goes, and the setup in the file takes its place.
    // A file that cannot be read at all leaves things as they are.
    void load_setup(const std::string& pathname) {
      Setup setup;
      std::vector<std::string> notes; // what could not be had, for the user
      try {
        setup = Setup::load(pathname, notes);
      } catch (const std::exception& e) {
        tell_user("The setup was not loaded.", e.what());
        return;
      }

      on_new();
      main_settings_dialog->set(setup.settings);

      std::vector<std::string> rejected;
      video_config_dialog->set(setup.video, rejected);
      for (const auto& what : rejected)
        notes.push_back("No such choice here: " + what + ". The default is used.");

      for (auto place : {GelAsset::Placement::left, GelAsset::Placement::right}) {
        const auto& gels = (place == GelAsset::Placement::left) ? setup.left : setup.right;
        for (const auto& gel : gels) {
          shared_ptr<Gui::GelAsset> ga;
          if (gel.kind == GelAsset::Kind::camera) {
            if (has_camera()) {
              notes.push_back("There can only be one camera gel; the second was left out.");
              continue;
            }
            ga = GelAsset::create_camera_asset(gel.device, place);
          } else {
            ga = GelAsset::create_gel_asset(gel.path, place);
            if (!ga) {
              notes.push_back("Could not open the gel " + gel.path + "; it was left out.");
              continue;
            }
          }
          add_asset(ga);
          ga->config().set_values(gel.rotate, gel.rate, gel.flip);
        }
      }
      refresh_active_gels();
      set_setup_pathname(pathname);
      cout << "Loaded the setup from " << pathname << endl;

      if (!notes.empty()) {
        std::string detail;
        for (const auto& note : notes)
          detail += note + "\n";
        tell_user("The setup was loaded, but not all of it.", detail);
      }
    }

    // The file the setup belongs to, which Save writes to. Its name
    // goes in the title of the window.
    void set_setup_pathname(const std::string& pathname) {
      setup_pathname = pathname;
      main_window->set_title(pathname.empty()
                             ? "Slit Scan"
                             : "Slit Scan - " + Glib::path_get_basename(pathname));
    }

    // Something the user has to be told, in a dialog of its own and
    // on the terminal.
    void tell_user(const std::string& message, const std::string& detail) {
      cerr << "SlitScan: " << message << " " << detail << endl;
      auto dialog = Gtk::AlertDialog::create(message);
      dialog->set_detail(detail);
      dialog->show(*main_window);
    }

    // RESTART: the scan starts over, exactly as it began. The
    // picture goes black, it is the turn of the first gel on each
    // side again, videos go back to their first frame and spinning
    // gels to where their spin started. The gels, the settings and
    // PLAY itself are left as they are: if it was playing it goes
    // straight on playing, from the top.
    void on_restart() {
      cout << "Restart clicked." << endl;

      slit_scan->discard_pending(); // ticks still waiting are from the old run
      slit_scan->clear();
      for (auto& ga : assets)
        ga->source().restart();
      first_turns();

      scan_frame.release();
      canvas->queue_draw();
      if (playing)
        slit_scan->resume();
      refresh_active_gels();
    }

    // It is the turn of the first gel on each side. The serials go
    // on counting, as they must never repeat.
    void first_turns() {
      for (Turn* turn : {&left_turn, &right_turn}) {
        turn->index = 0;
        ++turn->serial;
        turn->started = Gel::time_basis::now();
      }
    }

    // New: everything as it was when the application started. The
    // scan and the recording stop, the gels go, the picture goes,
    // and the settings and the video configuration are back to
    // what they start out as.
    void on_new() {
      cout << "New clicked." << endl;

      toggle_record->set_active(false); // finishes a recording properly
      toggle_play->set_active(false);

      for (auto& ga : assets) {
        ga->close_dialog();
        // The list wrapped the gel in a row of its own making when
        // it was appended. It is the row that has to go; taking
        // out just the gel would leave an empty row behind.
        auto& listbox = (ga->get_placement() == GelAsset::Placement::left) ? gel_left_listbox
                                                                           : gel_right_listbox;
        if (auto* row = ga->get_parent())
          listbox->remove(*row);
      }
      assets.clear();
      chosen_files.clear();
      first_turns();
      set_setup_pathname("");

      slit_scan->clear();
      scan_frame.release();
      canvas->queue_draw();

      main_settings_dialog->reset();
      video_config_dialog->reset();

      recording_to.clear();
      recording_how.clear();
      Glib::signal_idle().connect_once(
        sigc::track_obj([this] { toggle_record->set_tooltip_text(""); }, *this));
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

    bool has_camera() {
      for (auto& ga : assets)
        if (ga->get_kind() == GelAsset::Kind::camera)
          return true;
      return false;
    }

    void add_camera(GelAsset::Placement place) {
      if (has_camera()) {
        cout << "The camera is already a gel." << endl;
        return;
      }
      add_asset(GelAsset::create_camera_asset(0, place));
      refresh_active_gels();
    }

    Gtk::ListBox& listbox_of(GelAsset::Placement place) {
      return (place == GelAsset::Placement::left) ? *gel_left_listbox : *gel_right_listbox;
    }

    static Gel::SlitScan::Side side_of(GelAsset::Placement place) {
      return (place == GelAsset::Placement::left) ? Gel::SlitScan::Side::Left
                                                  : Gel::SlitScan::Side::Right;
    }

    // Puts a gel at the end of its list and hooks it up: its frames
    // to the scanner, its remove button, and dragging.
    void add_asset(shared_ptr<Gui::GelAsset> ga) {
      if (!ga)
        return;
      if (ga->get_placement() != GelAsset::Placement::left
          && ga->get_placement() != GelAsset::Placement::right)
        return;

      listbox_of(ga->get_placement()).append(*ga);

      // Which side a gel's frames are for is looked up when they
      // come, not fixed here: a gel can be dragged to the other side.
      GelAsset* gel = ga.get();
      ga->source().signal_next_frame().connect(
        sigc::bind(sigc::mem_fun(*this, &GelSelection::on_gel_frame), gel));
      // a video says when it has played through; see on_gel_finished()
      ga->source().signal_finished().connect(
        sigc::bind(sigc::mem_fun(*this, &GelSelection::on_gel_finished), gel));
      ga->signal_remove_requested().connect(
        sigc::bind(sigc::mem_fun(*this, &GelSelection::remove_gel), gel));

      // The gel can be picked up, and other gels dropped on it.
      // What travels is just the gel's number, as text.
      auto drag = Gtk::DragSource::create();
      drag->set_actions(Gdk::DragAction::MOVE);
      drag->signal_prepare().connect([gel](double, double) {
        Glib::Value<Glib::ustring> value;
        value.init(value.value_type());
        value.set(drag_prefix + std::to_string(gel->get_id()));
        return Gdk::ContentProvider::create(value);
      }, false);
      drag->signal_drag_begin().connect([drag, gel](const Glib::RefPtr<Gdk::Drag>&) {
        drag->set_icon(Gtk::WidgetPaintable::create(*gel), 0, 0); // the gel itself under the pointer
      });
      ga->add_controller(drag);
      add_drop_target(*ga, GelAsset::Placement::undefined, gel);

      assets.push_back(ga);
    }

    // what a dragged gel is called on its way
    static inline const std::string drag_prefix = "slitscan-gel:";

    // Lets gels be dropped on a widget. Dropped on another gel, the
    // dragged one takes that gel's place in its list (`onto`);
    // dropped on a list itself, it goes to the end of it (`place`).
    void add_drop_target(Gtk::Widget& widget, GelAsset::Placement place, GelAsset* onto) {
      auto drop = Gtk::DropTarget::create(G_TYPE_STRING, Gdk::DragAction::MOVE);
      drop->signal_drop().connect([this, place, onto](const Glib::ValueBase& value, double, double) {
        Glib::Value<Glib::ustring> text;
        text.init(value.gobj());
        const std::string what = text.get();
        if (what.rfind(drag_prefix, 0) != 0)
          return false; // some other text; not ours

        GelAsset* dragged = nullptr;
        for (auto& ga : assets)
          if (std::to_string(ga->get_id()) == what.substr(drag_prefix.size()))
            dragged = ga.get();
        if (!dragged || dragged == onto)
          return false;

        if (onto)
          move_gel(dragged, onto->get_placement(), position_of(onto));
        else
          move_gel(dragged, place, gels_of(place).size());
        return true;
      }, false);
      widget.add_controller(drop);
    }

    // where a gel is in the list of its side, from the top
    std::size_t position_of(GelAsset* gel) {
      const auto gels = gels_of(gel->get_placement());
      for (std::size_t i = 0; i < gels.size(); ++i)
        if (gels[i].get() == gel)
          return i;
      return gels.size();
    }

    // Takes the gel's row out of its list, leaving the gel itself
    // free to be put somewhere else. The list made the row when the
    // gel was appended; taking out just the gel would leave an
    // empty row behind.
    void take_from_list(GelAsset* gel) {
      if (auto* row = dynamic_cast<Gtk::ListBoxRow*>(gel->get_parent())) {
        row->unset_child();
        listbox_of(gel->get_placement()).remove(*row);
      }
    }

    // After the gels of a side have changed, it should still be the
    // turn of the gel it was (`was_active`), if that gel is still
    // there. If it is not, the gel that now stands in its place has
    // the turn, and a side left with no gels is taken out of the scan.
    void settle_turn(GelAsset::Placement place, const shared_ptr<Gui::GelAsset>& was_active) {
      const auto gels = gels_of(place);
      Turn& turn = turn_of(place);

      for (std::size_t i = 0; i < gels.size(); ++i) {
        if (gels[i] == was_active) {
          turn.index = i; // same gel, perhaps at another place in the list
          return;
        }
      }
      if (gels.empty()) {
        turn.index = 0;
        ++turn.serial;
        slit_scan->drop_side(side_of(place));
      } else {
        begin_turn(place, turn.index % gels.size());
      }
    }

    // Moves a gel to place number `position` of a side's list:
    // within its own list, or across to the other one.
    void move_gel(GelAsset* gel, GelAsset::Placement to, std::size_t position) {
      shared_ptr<Gui::GelAsset> ga;
      for (auto& candidate : assets)
        if (candidate.get() == gel)
          ga = candidate;
      if (!ga)
        return;

      const auto from = ga->get_placement();
      const auto active_left  = active_gel(GelAsset::Placement::left);
      const auto active_right = active_gel(GelAsset::Placement::right);

      // out of where it is ...
      take_from_list(gel);
      assets.remove(ga);

      // ... and in at its new place. `assets` holds both sides in
      // list order, so it goes in front of the gel now at that
      // place on that side, or at the end if there is none.
      ga->set_placement(to);
      const auto there = gels_of(to);
      position = std::min(position, there.size());
      if (position < there.size())
        assets.insert(std::find(assets.begin(), assets.end(), there[position]), ga);
      else
        assets.push_back(ga);
      listbox_of(to).insert(*ga, int(position));

      // The gel whose turn it was on each side keeps it. If that
      // was the gel just moved to the other side, its old side
      // goes on with the gel that stood below it.
      settle_turn(GelAsset::Placement::left,  (from != to && active_left == ga)  ? nullptr : active_left);
      settle_turn(GelAsset::Placement::right, (from != to && active_right == ga) ? nullptr : active_right);
      refresh_active_gels();
    }

    // Takes a gel out altogether.
    void remove_gel(GelAsset* gel) {
      shared_ptr<Gui::GelAsset> ga;
      for (auto& candidate : assets)
        if (candidate.get() == gel)
          ga = candidate;
      if (!ga)
        return;

      const auto place = ga->get_placement();
      const auto was_active = active_gel(place);
      const auto position = position_of(gel);

      ga->close_dialog();
      ga->source().set_active(false);
      take_from_list(gel);
      assets.remove(ga);

      // a gel above the one whose turn it is has gone: the turn
      // moves up with the gels below it
      Turn& turn = turn_of(place);
      if (was_active != ga && position < turn.index)
        --turn.index;
      settle_turn(place, was_active == ga ? nullptr : was_active);
      refresh_active_gels();

      // The gel is gone from the lists now, but this is its own
      // button's click we are in: let that finish before the gel
      // itself goes.
      Glib::signal_idle().connect_once([ga] {});
    }

    // What the scanner should be doing, from the global settings
    // and the video configuration.
    Gel::SlitScan::Params scan_params() {
      const auto& ms = obtain_main_settings();
      const auto& vc = VideoConfig::obtain_video_configiguration();

      Gel::SlitScan::Params params;
      params.width  = vc.res_width;
      params.height = vc.res_height;
      params.effect = vc.effect;
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

      // with several gels on a side, the slit crosses each one
      // once and then it is the next one's turn.
      params.sequence_left  = gels_of(GelAsset::Placement::left).size() > 1;
      params.sequence_right = gels_of(GelAsset::Placement::right).size() > 1;
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
        // the time a gel has had its turn for counts from now
        left_turn.started = right_turn.started = Gel::time_basis::now();
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
    void on_tick(Gel::time_stamp tick) {
      if (!playing)
        return;
      slit_scan->configure(scan_params());

      // a gel that has no end of its own gets a fixed time
      for (auto place : {GelAsset::Placement::left, GelAsset::Placement::right}) {
        if (is_endless(active_gel(place))
            && Gel::seconds_between(turn_of(place).started, tick) >= endless_gel_seconds)
          next_gel(place);
      }
    }

    // ---- Several gels on one side take turns, top to bottom and
    // round again. Whose turn it is ends in one of three ways:
    //
    //   + a still image: the slit has crossed all of it (the
    //     scanner says so; see on_pass_finished()).
    //   + a video: it has played through (see on_gel_finished()).
    //   + a camera, or a still image while the slit is not
    //     sweeping: neither ever ends, so they get
    //     endless_gel_seconds each (see on_tick()).
    //
    // Clicking a gel in its list makes it that gel's turn there and
    // then, and the round carries on from it.

    // how long a gel with no end of its own has, in seconds
    static constexpr double endless_gel_seconds = 10.0;

    // whose turn it is on a side
    struct Turn {
      std::size_t index = 0; // into the gels of that side, top to bottom
      unsigned serial = 1;   // goes up with every change; tells the scanner the gels apart
      Gel::time_stamp started = Gel::time_basis::now();
    };

    Turn& turn_of(GelAsset::Placement place) {
      return (place == GelAsset::Placement::left) ? left_turn : right_turn;
    }

    // the gels of a side, in the order of its list
    std::vector<shared_ptr<Gui::GelAsset>> gels_of(GelAsset::Placement place) {
      std::vector<shared_ptr<Gui::GelAsset>> gels;
      for (auto& ga : assets)
        if (ga->get_placement() == place)
          gels.push_back(ga);
      return gels;
    }

    // The gel being scanned on a side: the one whose turn it is.
    shared_ptr<Gui::GelAsset> active_gel(GelAsset::Placement place) {
      const auto gels = gels_of(place);
      if (gels.empty())
        return {};
      return gels[turn_of(place).index % gels.size()];
    }

    // does this gel go on for ever, as far as taking turns goes?
    bool is_endless(const shared_ptr<Gui::GelAsset>& ga) {
      if (!ga)
        return false;
      switch (ga->get_kind()) {
      case GelAsset::Kind::camera: return true;
      case GelAsset::Kind::video:  return false;
      default:                     return obtain_main_settings().sweep_rate == 0.0;
      }
    }

    // It is the turn of gel number `index` on a side.
    void begin_turn(GelAsset::Placement place, std::size_t index) {
      Turn& turn = turn_of(place);
      turn.index = index;
      ++turn.serial;
      turn.started = Gel::time_basis::now();
      refresh_active_gels();
    }

    // On to the next gel of a side, and from the last one back to
    // the first. With a single gel there is nowhere to go.
    void next_gel(GelAsset::Placement place) {
      const auto count = gels_of(place).size();
      if (count > 1)
        begin_turn(place, (turn_of(place).index + 1) % count);
    }

    // the slit has crossed the whole of a still image
    void on_pass_finished(Gel::SlitScan::Side side) {
      if (playing)
        next_gel(side == Gel::SlitScan::Side::Left ? GelAsset::Placement::left
                                                   : GelAsset::Placement::right);
    }

    // a video has played through
    void on_gel_finished(GelAsset* ga) {
      const auto place = ga->get_placement();
      if (playing && active_gel(place).get() == ga)
        next_gel(place);
    }

    // the user clicked a gel in a list: its turn, now
    void on_gel_selected(GelAsset::Placement place) {
      auto& listbox = (place == GelAsset::Placement::left) ? gel_left_listbox
                                                           : gel_right_listbox;
      const auto gels = gels_of(place);
      for (auto* row : listbox->get_selected_rows()) {
        for (std::size_t i = 0; i < gels.size(); ++i) {
          if (row && row->get_child() == gels[i].get()) {
            if (i != turn_of(place).index % gels.size())
              begin_turn(place, i);
            return;
          }
        }
      }
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

    // A frame from a gel, for the side the gel is on. The serial of
    // the turn goes with it, so that the scanner knows when it is
    // looking at another gel.
    void on_gel_frame(const cv::Mat& frame, Gel::time_stamp ts, GelAsset* gel) {
      const auto place = gel->get_placement();
      slit_scan->push_frame(side_of(place), frame, ts, turn_of(place).serial);
    }

    // a finished frame from the scanner
    void on_scan_frame(const cv::Mat& frame, Gel::time_stamp) {
      if (!playing)
        return; // a straggler from before PLAY went off, or New was clicked
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
    Turn left_turn;     // whose turn it is among the left gels
    Turn right_turn;    //                       and the right
    std::string recording_to; // the file being recorded, or last recorded
    std::string recording_how; // the codec and encoder writing it
    std::set<std::string> closing_recordings; // stopped, not yet closed

    unique_ptr<Gui::MainSettings> main_settings_dialog;
    unique_ptr<Gui::VideoConfig> video_config_dialog;
    
    shared_ptr<Gtk::Window> main_window;
    shared_ptr<Gtk::Button> restart_button;
    Glib::RefPtr<Gtk::FileChooserNative> setup_chooser; // while the user is choosing
    std::string setup_pathname; // the file this setup belongs to, if any
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
