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
//   video:
//     width: 1280
//     height: 800
//     fps: 30
//     format: mp4
//     codec: h264          # h264, h265, vp9, av1
//     effect: vertical     # vertical, horizontal
//     output: ""           # where REC writes; empty for ~/Videos
//   gels:
//     left:
//       - kind: image      # image, video, camera
//         path: /home/me/gels/stripes.png
//         rotate: 0        # degrees
//         rate: 0          # revolutions per GelConfig::rate_minutes
//         flip: false
//       - kind: camera
//         device: 0
//     right: []
//
// Anything missing from a file takes its default, so files written
// by an older version still load.
#pragma once

#include <fstream>
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

    // ---- writing

    // Writes the setup to a file. Throws std::runtime_error, with
    // words fit for the user, if that cannot be done.
    void save(const std::string& pathname) const {
      YAML::Emitter out;
      out << YAML::BeginMap;
      out << YAML::Key << "slitscan" << YAML::Value << file_version;

      out << YAML::Key << "settings" << YAML::Value << YAML::BeginMap;
      out << YAML::Key << "slit_width"         << YAML::Value << settings.slit_width;
      out << YAML::Key << "slit_spacing"       << YAML::Value << settings.slit_spacing;
      out << YAML::Key << "perspective_factor" << YAML::Value << settings.perspective_factor;
      out << YAML::Key << "sweep_rate"         << YAML::Value << settings.sweep_rate;
      out << YAML::EndMap;

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
            out << YAML::Key << "path" << YAML::Value << YAML::DoubleQuoted << gel.path;
          out << YAML::Key << "rotate" << YAML::Value << gel.rotate;
          out << YAML::Key << "rate"   << YAML::Value << gel.rate;
          out << YAML::Key << "flip"   << YAML::Value << gel.flip;
          out << YAML::EndMap;
        }
        out << YAML::EndSeq;
      }
      out << YAML::EndMap;
      out << YAML::EndMap;

      std::ofstream file(pathname, std::ios::trunc);
      file << "# Slit Scan setup\n" << out.c_str() << "\n";
      file.close();
      if (!file)
        throw std::runtime_error("Could not write " + pathname);
    }

    // ---- reading

    // Reads a setup from a file. Throws std::runtime_error, with
    // words fit for the user, if the file cannot be read or is not
    // a setup at all. Things in it that we can make nothing of are
    // left at their defaults and noted in `warnings`.
    static Setup load(const std::string& pathname, std::vector<std::string>& warnings) {
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

      Setup setup;
      try {
        if (root["slitscan"].as<int>(file_version) > file_version)
          warnings.push_back("This file was written by a newer Slit Scan; "
                             "some of it may be ignored.");

        if (const YAML::Node s = root["settings"]; s && s.IsMap()) {
          setup.settings.slit_width         = s["slit_width"].as<int>(setup.settings.slit_width);
          setup.settings.slit_spacing       = s["slit_spacing"].as<int>(setup.settings.slit_spacing);
          setup.settings.perspective_factor = s["perspective_factor"].as<double>(setup.settings.perspective_factor);
          setup.settings.sweep_rate         = s["sweep_rate"].as<double>(setup.settings.sweep_rate);
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
              gel.path   = node["path"].as<std::string>("");
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
