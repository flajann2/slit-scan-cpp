// Video Configuration Dialog
#pragma once

#include <VideoEncoding.h>
#include <ScanEffect.h>

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

  // ComboBox order must match
  constexpr auto create_effect_list() {
    std::array<Gel::ScanEffect, 2> vec {{   Gel::ScanEffect::VerticalSlit
                                          , Gel::ScanEffect::HorizontalSlit }};
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
      Gel::ScanEffect effect; // what the scanner makes of the gels
    };
    
    // what the configuration is when the application starts
    static Configuration defaults() {
      return {"", 1280, 800, 30, "mp4", Gel::VideoCodec::H264
              , Gel::ScanEffect::VerticalSlit};
    }

    VideoConfig () {
      video_configuration = defaults();
      auto builder = obtain_builder();

      // widgets
      video_config_dialog = builder->get_object<Gtk::Dialog>(ss::video_config);
      float_over_main_window(*video_config_dialog);
      resolution = builder->get_object<Gtk::ComboBox>(ss::video_p1_resolution);
      vid_fps    = builder->get_object<Gtk::ComboBox>(ss::video_p1_fps);
      vid_format = builder->get_object<Gtk::ComboBox>(ss::video_p1_container);
      vid_codec  = builder->get_object<Gtk::ComboBox>(ss::video_p1_codec);
      vid_effect = builder->get_object<Gtk::ComboBox>(ss::video_p2_effect);

      // set the initial (default) config to match with the initial config
      show_defaults();
      
      // signals and slots
      resolution->signal_changed().connect(sigc::mem_fun(*this, &VideoConfig::on_resolution_changed));
      vid_fps->signal_changed().connect(   sigc::mem_fun(*this, &VideoConfig::on_fps_changed));
      vid_format->signal_changed().connect(sigc::mem_fun(*this, &VideoConfig::on_format_changed));
      vid_codec->signal_changed().connect( sigc::mem_fun(*this, &VideoConfig::on_codec_changed));
      vid_effect->signal_changed().connect(sigc::mem_fun(*this, &VideoConfig::on_effect_changed));
    }
    
    static Configuration video_configuration;

    // Back to the configuration the application starts with.
    // Setting the boxes runs their handlers, which see to the
    // configuration itself.
    void reset() {
      show_defaults();
      video_configuration = defaults();
    }

    // This is the configuration now, in the dialog as well. A value
    // the dialog has no row for (a resolution it does not list,
    // say) is left as it was, and named in `rejected`.
    void set(const Configuration& wanted, std::vector<std::string>& rejected) {
      // each box is set to the row that holds the value; setting it
      // runs the box's handler, which sees to the configuration.
      auto choose = [&](shared_ptr<Gtk::ComboBox>& box, int row, const std::string& what) {
        if (row >= 0)
          box->set_active(row);
        else
          rejected.push_back(what);
      };
      auto row_of = [](const auto& list, const auto& matches) {
        for (int row = 0; row < int(list.size()); ++row)
          if (matches(list[row]))
            return row;
        return -1;
      };

      choose(resolution
             , row_of(create_resolution_list(), [&](const auto& r) {
                 return r.first == wanted.res_width && r.second == wanted.res_height; })
             , std::format("resolution {}x{}", wanted.res_width, wanted.res_height));
      choose(vid_fps
             , row_of(create_fps_list(), [&](int fps) { return fps == int(wanted.frames_per_sec); })
             , std::format("{} frames per second", wanted.frames_per_sec));
      choose(vid_format
             , row_of(create_format_list(), [&](const char* f) { return wanted.format == f; })
             , "file format " + wanted.format);
      choose(vid_codec
             , row_of(create_codec_list(), [&](Gel::VideoCodec c) { return c == wanted.codec; })
             , std::string("codec ") + Gel::codec_name(wanted.codec));
      choose(vid_effect
             , row_of(create_effect_list(), [&](Gel::ScanEffect e) { return e == wanted.effect; })
             , std::string("effect ") + Gel::scan_effect_name(wanted.effect));

      video_configuration.pathnname = wanted.pathnname;
    }
    
  protected:
    // the boxes at the rows that go with defaults()
    void show_defaults() {
      resolution->set_active(2); // 1280x800
      vid_fps->set_active(2);    // 30
      vid_format->set_active(0); // mp4
      vid_codec->set_active(0);  // H.264
      vid_effect->set_active(0); // vertical slit scan
    }

    shared_ptr<Gtk::Dialog> video_config_dialog;
    shared_ptr<Gtk::ComboBox> resolution;
    shared_ptr<Gtk::ComboBox> vid_fps;
    shared_ptr<Gtk::ComboBox> vid_format;
    shared_ptr<Gtk::ComboBox> vid_codec;
    shared_ptr<Gtk::ComboBox> vid_effect;

    
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

    // The effect takes hold at once, also while playing: the
    // scanner picks the configuration up on its next tick.
    void on_effect_changed() {
      constexpr auto list = create_effect_list();
      int row = vid_effect->get_active_row_number();
      if (row < 0 || row >= int(list.size()))
        return;
      video_configuration.effect = list[row];

      cout << "effect row: " << row << " as "
           << Gel::scan_effect_name(video_configuration.effect)
           << endl;
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
