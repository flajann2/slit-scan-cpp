// Setup.h -- a whole setup, and its YAML file.
//
// Everything Load and Save are about: the settings, the video
// configuration, and the gels of each side with their full
// pathnames and what is set in their dialogs. This is plain data
// and file handling; putting a setup on screen, and taking one off
// it, is GelSelection's business.
//
// The file looks like this:
//
//   slitscan: 1
//   settings:
//     slit_width: 3
//     slit_spacing: 0
//     perspective_factor: 10
//     sweep_rate: 0.05
//     video_folder: ""     # where REC writes; empty for ~/Videos
//   video:
//     width: 1280
//     height: 800
//     fps: 30
//     format: mp4
//     codec: h264          # h264, h265, vp9, av1
//     effect: vertical     # vertical, horizontal
//     output: ""           # the very file REC writes; empty for a new
//                          # one each time, in the settings' video_folder
//   gels:
//     left:
//       - kind: image      # image, video, camera
//         path: gels/stripes.png
//         rotate: 0        # degrees
//         rate: 0          # revolutions per GelConfig::rate_minutes
//         flip: false
//       - kind: camera
//         device: 0
//     right: []
//
// A gel in the setup file's own folder, or below it, is written with
// a path relative to that folder, and a relative path is read as
// relative to it. So a folder holding a setup and its gels can be
// moved, copied or shared as it is. A gel anywhere else is written
// with its full pathname.
//
// Anything missing from a file takes its default, so files written
// by an older version still load. The `settings` are different:
// they are laid over the user's own (see Settings.h), so what the
// file says overrides those and what it leaves out stays as the
// user has it.
#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace Gui {

  struct Setup {
    // one gel, as the file has it
    struct GelEntry {
      GelAsset::Kind kind = GelAsset::Kind::image;
      std::string path;    // images and videos: the full pathname
      int device = 0;      // cameras: OpenCV's number for the device
      double rotate = 0.0; // as in the gel's dialog
      double rate = 0.0;
      bool flip = false;
    };

    static constexpr int file_version = 1;

    MainSettings::Settings settings = MainSettings::defaults();
    VideoConfig::Configuration video = VideoConfig::defaults();
    std::vector<GelEntry> left;
    std::vector<GelEntry> right;

    // ---- the words used in the file

    static const char* kind_word(GelAsset::Kind kind) {
      switch (kind) {
      case GelAsset::Kind::video:  return "video";
      case GelAsset::Kind::camera: return "camera";
      default:                     return "image";
      }
    }

    static const char* codec_word(Gel::VideoCodec codec) {
      switch (codec) {
      case Gel::VideoCodec::H265: return "h265";
      case Gel::VideoCodec::VP9:  return "vp9";
      case Gel::VideoCodec::AV1:  return "av1";
      default:                      return "h264";
      }
    }

    static const char* effect_word(Gel::ScanEffect effect) {
      return effect == Gel::ScanEffect::HorizontalSlit ? "horizontal" : "vertical";
    }

    // ---- paths

    // How a gel's path is written in a setup file kept in `folder`:
    // relative to it if the gel is in it or below it, and otherwise
    // in full. Always with forward slashes, which every system reads.
    static std::string path_for_file(const std::string& path, const std::filesystem::path& folder) {
      namespace fs = std::filesystem;
      const fs::path gel(path);
      if (gel.is_relative() || folder.empty())
        return gel.generic_string();

      const fs::path relative = gel.lexically_normal().lexically_relative(folder.lexically_normal());
      if (relative.empty() || *relative.begin() == "..")
        return gel.generic_string(); // somewhere else: in full
      return relative.generic_string();
    }

    // and the other way: a path from a setup file kept in `folder`
    static std::string path_from_file(const std::string& path, const std::filesystem::path& folder) {
      namespace fs = std::filesystem;
      const fs::path gel(path);
      if (path.empty() || gel.is_absolute() || folder.empty())
        return path;
      return (folder / gel).lexically_normal().string();
    }

    // ---- writing

    // The setup as the text of a setup file that will be kept in
    // `folder`.
    std::string to_yaml(const std::filesystem::path& folder) const {
      YAML::Emitter out;
      out.SetDoublePrecision(15); // 0.05, not 0.050000000000000003
      out << YAML::BeginMap;
      out << YAML::Key << "slitscan" << YAML::Value << file_version;

      out << YAML::Key << "settings" << YAML::Value;
      settings.write(out);

      out << YAML::Key << "video" << YAML::Value << YAML::BeginMap;
      out << YAML::Key << "width"  << YAML::Value << video.res_width;
      out << YAML::Key << "height" << YAML::Value << video.res_height;
      out << YAML::Key << "fps"    << YAML::Value << video.frames_per_sec;
      out << YAML::Key << "format" << YAML::Value << video.format;
      out << YAML::Key << "codec"  << YAML::Value << codec_word(video.codec);
      out << YAML::Key << "effect" << YAML::Value << effect_word(video.effect);
      out << YAML::Key << "output" << YAML::Value << YAML::DoubleQuoted << video.pathnname;
      out << YAML::EndMap;

      out << YAML::Key << "gels" << YAML::Value << YAML::BeginMap;
      for (const auto& [name, gels] : {std::pair{"left", &left}, std::pair{"right", &right}}) {
        out << YAML::Key << name << YAML::Value << YAML::BeginSeq;
        for (const GelEntry& gel : *gels) {
          out << YAML::BeginMap;
          out << YAML::Key << "kind" << YAML::Value << kind_word(gel.kind);
          if (gel.kind == GelAsset::Kind::camera)
            out << YAML::Key << "device" << YAML::Value << gel.device;
          else
            out << YAML::Key << "path" << YAML::Value << YAML::DoubleQuoted
                << path_for_file(gel.path, folder);
          out << YAML::Key << "rotate" << YAML::Value << gel.rotate;
          out << YAML::Key << "rate"   << YAML::Value << gel.rate;
          out << YAML::Key << "flip"   << YAML::Value << gel.flip;
          out << YAML::EndMap;
        }
        out << YAML::EndSeq;
      }
      out << YAML::EndMap;
      out << YAML::EndMap;

      return std::string("# Slit Scan setup\n") + out.c_str() + "\n";
    }

    // Writes the setup to a file. Throws std::runtime_error, with
    // words fit for the user, if that cannot be done.
    void save(const std::string& pathname) const {
      const auto folder = std::filesystem::absolute(pathname).parent_path();
      std::ofstream file(pathname, std::ios::trunc | std::ios::binary);
      file << to_yaml(folder);
      file.close();
      if (!file)
        throw std::runtime_error("Could not write " + pathname);
    }

    // ---- reading

    // Reads a setup from a file. Throws std::runtime_error, with
    // words fit for the user, if the file cannot be read or is not
    // a setup at all. Things in it that we can make nothing of are
    // left at their defaults and noted in `warnings`.
    //
    // `user_settings` are the user's own settings. The file's are
    // laid over them: what it says wins, and what it does not
    // mention stays as the user has it.
    static Setup load(const std::string& pathname, std::vector<std::string>& warnings
                      , const MainSettings::Settings& user_settings = MainSettings::defaults()) {
      YAML::Node root;
      try {
        root = YAML::LoadFile(pathname);
      } catch (const YAML::BadFile&) {
        throw std::runtime_error("Could not read " + pathname);
      } catch (const YAML::Exception& e) {
        throw std::runtime_error(pathname + " is not valid YAML: " + e.what());
      }
      if (!root.IsMap() || !root["slitscan"])
        throw std::runtime_error(pathname + " is not a Slit Scan setup.");

      // relative gel paths are relative to the file's own folder
      const auto folder = std::filesystem::absolute(pathname).parent_path();

      Setup setup;
      setup.settings = user_settings;
      try {
        if (root["slitscan"].as<int>(file_version) > file_version)
          warnings.push_back("This file was written by a newer Slit Scan; "
                             "some of it may be ignored.");

        setup.settings.read(root["settings"], warnings);

        // A folder for recordings that is not on this computer (the
        // setup came from someone else's) is no use here, and is not
        // one to be made behind the user's back either.
        if (setup.settings.video_folder != user_settings.video_folder
            && !setup.settings.video_folder.empty()
            && !Glib::file_test(Paths::expanded(setup.settings.video_folder), Glib::FileTest::IS_DIR)) {
          warnings.push_back("The setup's folder for recordings, " + setup.settings.video_folder
                             + ", is not on this computer; yours is used.");
          setup.settings.video_folder = user_settings.video_folder;
        }

        if (const YAML::Node v = root["video"]; v && v.IsMap()) {
          setup.video.res_width      = v["width"].as<int>(setup.video.res_width);
          setup.video.res_height     = v["height"].as<int>(setup.video.res_height);
          setup.video.frames_per_sec = v["fps"].as<double>(setup.video.frames_per_sec);
          setup.video.format         = v["format"].as<std::string>(setup.video.format);
          setup.video.pathnname      = v["output"].as<std::string>(setup.video.pathnname);

          const auto codec = v["codec"].as<std::string>(codec_word(setup.video.codec));
          bool known = false;
          for (auto c : create_codec_list())
            if (codec == codec_word(c)) { setup.video.codec = c; known = true; }
          if (!known)
            warnings.push_back("Unknown codec \"" + codec + "\"; using "
                               + codec_word(setup.video.codec) + ".");

          const auto effect = v["effect"].as<std::string>(effect_word(setup.video.effect));
          known = false;
          for (auto e : create_effect_list())
            if (effect == effect_word(e)) { setup.video.effect = e; known = true; }
          if (!known)
            warnings.push_back("Unknown effect \"" + effect + "\"; using "
                               + effect_word(setup.video.effect) + ".");
        }

        if (const YAML::Node g = root["gels"]; g && g.IsMap()) {
          for (const auto& [name, gels] : {std::pair{"left", &setup.left}, std::pair{"right", &setup.right}}) {
            const YAML::Node list = g[name];
            if (!list || !list.IsSequence())
              continue;
            for (const YAML::Node& node : list) {
              if (!node.IsMap())
                continue;
              GelEntry gel;
              const auto kind = node["kind"].as<std::string>("image");
              if (kind == "camera")     gel.kind = GelAsset::Kind::camera;
              else if (kind == "video") gel.kind = GelAsset::Kind::video;
              else                      gel.kind = GelAsset::Kind::image;
              gel.path   = path_from_file(node["path"].as<std::string>(""), folder);
              gel.device = node["device"].as<int>(0);
              gel.rotate = node["rotate"].as<double>(0.0);
              gel.rate   = node["rate"].as<double>(0.0);
              gel.flip   = node["flip"].as<bool>(false);
              gels->push_back(gel);
            }
          }
        }
      } catch (const YAML::Exception& e) {
        throw std::runtime_error(pathname + " has something in it that is not what "
                                 "a setup should have: " + e.what());
      }
      return setup;
    }
  };
}
