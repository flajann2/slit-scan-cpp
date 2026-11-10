// Video Configuration Dialog
#pragma once

#include <VideoEncoding.h>

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

  // ComboBox order must match
  constexpr auto create_codec_list() {
    std::array<Gel::VideoCodec, 4> vec {{   Gel::VideoCodec::H264
                                          , Gel::VideoCodec::H265
                                          , Gel::VideoCodec::VP9
                                          , Gel::VideoCodec::AV1 }};
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
      Gel::VideoCodec codec; // what the video is encoded with
    };
    
    VideoConfig () {
      video_configuration = {"", 1280, 800, 30, "mp4", Gel::VideoCodec::H264};
      auto builder = obtain_builder();

      // widgets
      video_config_dialog = builder->get_object<Gtk::Dialog>(ss::video_config);
      resolution = builder->get_object<Gtk::ComboBox>(ss::video_p1_resolution);
      vid_fps    = builder->get_object<Gtk::ComboBox>(ss::video_p1_fps);
      vid_format = builder->get_object<Gtk::ComboBox>(ss::video_p1_container);
      vid_codec  = builder->get_object<Gtk::ComboBox>(ss::video_p1_codec);

      // set the initial (default) config to match with the initial config
      resolution->set_active(2); // 1280x800
      vid_fps->set_active(2);    // 30
      vid_format->set_active(0); // mp4
      vid_codec->set_active(0);  // H.264
      
      // signals and slots
      resolution->signal_changed().connect(sigc::mem_fun(*this, &VideoConfig::on_resolution_changed));
      vid_fps->signal_changed().connect(   sigc::mem_fun(*this, &VideoConfig::on_fps_changed));
      vid_format->signal_changed().connect(sigc::mem_fun(*this, &VideoConfig::on_format_changed));
      vid_codec->signal_changed().connect( sigc::mem_fun(*this, &VideoConfig::on_codec_changed));
    }
    
    static Configuration video_configuration;
    
  protected:
    shared_ptr<Gtk::Dialog> video_config_dialog;
    shared_ptr<Gtk::ComboBox> resolution;
    shared_ptr<Gtk::ComboBox> vid_fps;
    shared_ptr<Gtk::ComboBox> vid_format;
    shared_ptr<Gtk::ComboBox> vid_codec;

    
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

    // Not every file format can hold every codec: webm takes VP9
    // and AV1 only, mov will not take those two, and so on. So the
    // two boxes keep each other honest. Change one to something the
    // other cannot go with, and the other moves to what does.

    void on_format_changed() {
      constexpr auto list = create_format_list();
      int row = vid_format->get_active_row_number();
      if (row < 0 || row >= int(list.size()))
        return;
      video_configuration.format = std::string(list[row]);

      cout << "format row: " << row << " as "
           << video_configuration.format
           << endl;

      // ogg holds none of the four codecs on offer; it is recorded
      // as Theora, whatever the codec box says.
      if (!Gel::container_holds(video_configuration.format, video_configuration.codec))
        select_codec(Gel::default_codec(video_configuration.format));
    }

    void on_codec_changed() {
      constexpr auto list = create_codec_list();
      int row = vid_codec->get_active_row_number();
      if (row < 0 || row >= int(list.size()))
        return;
      video_configuration.codec = list[row];

      cout << "codec row: " << row << " as "
           << Gel::codec_name(video_configuration.codec)
           << endl;

      if (!Gel::container_holds(video_configuration.format, video_configuration.codec))
        select_format(Gel::natural_container(video_configuration.codec));
    }

    // Set a box to a value, if the box has it. Setting it runs the
    // box's changed handler, which brings the configuration along.
    void select_codec(Gel::VideoCodec codec) {
      constexpr auto list = create_codec_list();
      for (int row = 0; row < int(list.size()); ++row)
        if (list[row] == codec && vid_codec->get_active_row_number() != row)
          vid_codec->set_active(row);
    }

    void select_format(const std::string& format) {
      constexpr auto list = create_format_list();
      for (int row = 0; row < int(list.size()); ++row)
        if (format == list[row] && vid_format->get_active_row_number() != row)
          vid_format->set_active(row);
    }
    
  public:
    static const VideoConfig::Configuration& obtain_video_configiguration(){ return video_configuration; }
    static unique_ptr<VideoConfig>create() { return make_unique<VideoConfig>(); }

    void show () { video_config_dialog->show(); }
    void hide () { video_config_dialog->hide(); }
  };
}
