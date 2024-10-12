// Video Configuration Dialog
#pragma once

namespace Gui {

  // ComboBox order must match
  constexpr auto create_resolution_list() {
    std::array<std::pair<int, int>, 12> vec {{
        {800  , 600 },
        {1024 , 768 },
        {1280 , 800 },
        {1280 , 720 },
        {1280 , 1024},
        {1366 , 768 },
        {1920 , 1080},
        {1920 , 1200},
        {2560 , 1440},
        {3840 , 2160},
        {5120 , 2880},
        {7680 , 4320},
      }};
    return vec;
  }

  // ComboBox order must match
  constexpr auto create_fps_list() {
    std::array<int, 5> vec {{ 24, 25, 30, 50, 60, }};
    return vec;
  }

  // ComboBox order must match
  constexpr auto create_format_list() {
    std::array<const char*, 6> vec {{     "mp4"
                                        , "avi"
                                        , "mov"
                                        , "webm"
                                        , "mkv"
                                        , "ogg" }};
    return vec;
  }

  class VideoConfig : public Composition {
  public:
    struct Configuration {
      std::string pathnname; // extension will sort of define the type.

      int res_width;
      int res_height;
      double frames_per_sec; // TODO: we have this defined in global settings.
      std::string format; // mp4, mkv, etc.
    };
    
    VideoConfig () {
      video_configuration = {"", 1280, 800, 30, "mp4"};
      auto builder = obtain_builder();

      // widgets
      video_config_dialog = builder->get_object<Gtk::Dialog>(ss::video_config);
      resolution = builder->get_object<Gtk::ComboBox>(ss::video_p1_resolution);
      vid_fps    = builder->get_object<Gtk::ComboBox>(ss::video_p1_fps);
      vid_format = builder->get_object<Gtk::ComboBox>(ss::video_p1_container);

      // set the initial (default) config to match with the initial config
      resolution->set_active(2); // 1280x800
      vid_fps->set_active(2);    // 30
      vid_format->set_active(0); // mp5
      
      // signals and slots
      resolution->signal_changed().connect(sigc::mem_fun(*this, &VideoConfig::on_resolution_changed));
      vid_fps->signal_changed().connect(   sigc::mem_fun(*this, &VideoConfig::on_fps_changed));
      vid_format->signal_changed().connect(sigc::mem_fun(*this, &VideoConfig::on_format_changed));
    }
    
    static Configuration video_configuration;
    
  protected:
    shared_ptr<Gtk::Dialog> video_config_dialog;
    shared_ptr<Gtk::ComboBox> resolution;
    shared_ptr<Gtk::ComboBox> vid_fps;
    shared_ptr<Gtk::ComboBox> vid_format;

    
    void on_resolution_changed() {
      constexpr auto list = create_resolution_list();
      int row = resolution->get_active_row_number();
      auto [w, h] = list[row];
      video_configuration.res_width = w;
      video_configuration.res_height = h;
      
      cout << "res row: " << row << " as ["
           << video_configuration.res_width << ","
           << video_configuration.res_height << "]" << endl;
    }

    void on_fps_changed() {
      constexpr auto list = create_fps_list();
      int row = vid_fps->get_active_row_number();
      video_configuration.frames_per_sec = list[row];

      cout << "fps row: " << row << " as "
           << video_configuration.frames_per_sec << endl;
    }

    void on_format_changed() {
      constexpr auto list = create_format_list();
      int row = vid_format->get_active_row_number();
      video_configuration.format = std::string(list[row]);

      cout << "format row: " << row << " as "
           << video_configuration.format
           << endl;
    }
    
  public:
    static const VideoConfig::Configuration& obtain_video_configiguration(){ return video_configuration; }
    static unique_ptr<VideoConfig>create() { return make_unique<VideoConfig>(); }

    void show () { video_config_dialog->show(); }
    void hide () { video_config_dialog->hide(); }
  };
}
