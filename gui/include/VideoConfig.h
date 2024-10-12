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
    std::vector<int> vec;
    vec.reserve(5);
    vec.emplace_back(24);
    vec.emplace_back(25);
    vec.emplace_back(30);
    vec.emplace_back(50);
    vec.emplace_back(60);
    return vec;
  }

  // ComboBox order must match
  constexpr auto create_format_list() {
    std::vector<std::string> vec;
    vec.reserve(6);
    vec.emplace_back("mp4");
    vec.emplace_back("avi");
    vec.emplace_back("mov");
    vec.emplace_back("webm");
    vec.emplace_back("mkv");
    vec.emplace_back("ogg");
    return vec;
  }

  class VideoConfig : public Composition {
  public:
    struct Configuration {
      std::string pathnname; // extension will sort of define the type.
      int res_width;
      int res_height;
      double frames_per_sec; // TODO: we have this defined in global settings.

      //static const constexpr auto reslist = create_resolution_list();
      //static const auto fpslist = create_fps_list();
    };
    
    VideoConfig () {
      auto builder = obtain_builder();

      // widgets
      video_config_dialog = builder->get_object<Gtk::Dialog>(ss::video_config);
      resolution = builder->get_object<Gtk::ComboBox>(ss::video_p1_resolution);
      vid_fps    = builder->get_object<Gtk::ComboBox>(ss::video_p1_fps);
      vid_format = builder->get_object<Gtk::ComboBox>(ss::video_p1_container);

      // signals and slots
      resolution->signal_changed().connect(sigc::mem_fun(*this, &VideoConfig::on_resolution_changed));
      vid_fps->signal_changed().connect(   sigc::mem_fun(*this, &VideoConfig::on_fps_changed));
      vid_format->signal_changed().connect(sigc::mem_fun(*this, &VideoConfig::on_format_changed));
    }
    
  protected:
    shared_ptr<Gtk::Dialog> video_config_dialog;
    shared_ptr<Gtk::ComboBox> resolution;
    shared_ptr<Gtk::ComboBox> vid_fps;
    shared_ptr<Gtk::ComboBox> vid_format;

    
  private:

    void on_resolution_changed() {
      constexpr auto list = create_resolution_list();
      int row = resolution->get_active_row_number();
      auto [w, h] = list[row];
      cout << "res row: " << row << " as [" << w << "," << h << "]" << endl;
    }

    void on_fps_changed() {
      int row = vid_fps->get_active_row_number();
      cout << "fps row: " << row << endl;
    }

    void on_format_changed() {
      int row = vid_format->get_active_row_number();
      cout << "format row: " << row << endl;
    }



    
  public:
    static Configuration video_configuration;
    static const VideoConfig::Configuration& obtain_video_configiguration(){ return video_configuration; }
    static unique_ptr<VideoConfig>create() { return make_unique<VideoConfig>(); }

    void show () { video_config_dialog->show(); }
    void hide () { video_config_dialog->hide(); }
  };
}
