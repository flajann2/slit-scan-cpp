// Settings.h of SlitScan -- what the Settings dialog sets, and the
// file it is kept in.
//
// The settings belong to the user, not to a setup: they are read
// when Slit Scan starts and written when one is changed in the
// dialog, to the file Paths::config_file() names
// (~/.config/slitscan/config.yaml). The file looks like this:
//
//   slitscan_config: 1
//   settings:
//     slit_width: 3
//     slit_spacing: 0
//     perspective_factor: 10
//     sweep_rate: 0.05
//     video_folder: ""     # where REC writes; empty for ~/Videos
//
// A setup file has the same `settings` map (see Setup.h). When a
// setup is loaded or imported, what it says overrides the user's
// settings, for as long as the setup is in use; what it does not
// mention stays as the user has it. The user's file is not touched
// by that. It changes only when the user changes something in the
// dialog, and then only in what was changed: see take_changes().
//
// To add a setting: a member here, a line each in write(), read()
// and take_changes(), and its widget in MainSettings.h.
//
// This is plain data and file handling, with nothing of the GUI in
// it.
#pragma once

#include <fstream>

namespace Gui {
  struct Settings {
    int slit_width = 3;               // width of each slit
    int slit_spacing = 0;             // space between slits
    double perspective_factor = 10.0;
    double sweep_rate = 0.05;         // of the slit across a still gel, in gel widths per second
    std::string video_folder;         // where recordings go; empty for Paths::default_video_folder()

    bool operator==(const Settings&) const = default;

    // what the settings are with no file to say otherwise
    static Settings defaults() { return Settings{}; }

    // ---- as YAML: the `settings` map of a config file or a setup file

    void write(YAML::Emitter& out) const {
      out << YAML::BeginMap;
      out << YAML::Key << "slit_width"         << YAML::Value << slit_width;
      out << YAML::Key << "slit_spacing"       << YAML::Value << slit_spacing;
      out << YAML::Key << "perspective_factor" << YAML::Value << perspective_factor;
      out << YAML::Key << "sweep_rate"         << YAML::Value << sweep_rate;
      out << YAML::Key << "video_folder"       << YAML::Value << YAML::DoubleQuoted << video_folder;
      out << YAML::EndMap;
    }

    // Takes what the map has. What it does not mention is left as
    // it is, so the result is the map's settings laid over the ones
    // that were here. A value that is not what it should be (a
    // word where a number belongs) is left as it is too, and noted
    // in `notes`.
    void read(const YAML::Node& map, std::vector<std::string>& notes) {
      if (!map || !map.IsMap())
        return;
      take(map, "slit_width", slit_width, notes);
      take(map, "slit_spacing", slit_spacing, notes);
      take(map, "perspective_factor", perspective_factor, notes);
      take(map, "sweep_rate", sweep_rate, notes);
      take(map, "video_folder", video_folder, notes);
    }

  private:
    // one value from the map, if it is there and is what it should be
    template <typename T>
    static void take(const YAML::Node& map, const char* key, T& field
                     , std::vector<std::string>& notes) {
      const YAML::Node node = map[key];
      if (!node)
        return; // not mentioned
      T value{};
      if (node.IsScalar() && YAML::convert<T>::decode(node, value))
        field = value;
      else
        notes.push_back(std::format("{}: {} is not a value it can have; it is left at {}."
                                    , key, node.IsScalar() ? "\"" + node.Scalar() + "\"" : "that"
                                    , field));
    }

  public:
    // Takes from `after` whatever is not as it was in `before`, and
    // nothing else. This is how an edit in the dialog reaches the
    // user's own settings: only what was edited, and not what a
    // setup had laid over them.
    void take_changes(const Settings& before, const Settings& after) {
      if (after.slit_width != before.slit_width)                 slit_width = after.slit_width;
      if (after.slit_spacing != before.slit_spacing)             slit_spacing = after.slit_spacing;
      if (after.perspective_factor != before.perspective_factor) perspective_factor = after.perspective_factor;
      if (after.sweep_rate != before.sweep_rate)                 sweep_rate = after.sweep_rate;
      if (after.video_folder != before.video_folder)             video_folder = after.video_folder;
    }
  };

  // The user's settings file.
  struct Config {
    static constexpr int file_version = 1;

    // The settings as the text of the file.
    static std::string to_yaml(const Settings& settings) {
      YAML::Emitter out;
      out.SetDoublePrecision(15); // 0.05, not 0.050000000000000003
      out << YAML::BeginMap;
      out << YAML::Key << "slitscan_config" << YAML::Value << file_version;
      out << YAML::Key << "settings" << YAML::Value;
      settings.write(out);
      out << YAML::EndMap;
      return std::string("# Slit Scan settings, as set in the Settings dialog\n") + out.c_str() + "\n";
    }

    // The user's settings: the defaults, with what the file says
    // laid over them. No file is no trouble, it is how things are
    // before the first change. A file that cannot be made sense of
    // is left alone and noted in `warnings`, and the defaults are
    // used.
    static Settings load(std::vector<std::string>& warnings
                         , const std::string& pathname = Paths::config_file()) {
      Settings settings = Settings::defaults();
      std::error_code ec;
      if (!std::filesystem::exists(pathname, ec))
        return settings;
      try {
        const YAML::Node root = YAML::LoadFile(pathname);
        if (!root.IsMap())
          throw std::runtime_error("it is not a map");
        if (root["slitscan_config"].as<int>(file_version) > file_version)
          warnings.push_back(pathname + " was written by a newer Slit Scan; "
                             "some of it may be ignored.");
        settings.read(root["settings"], warnings);
      } catch (const std::exception& e) {
        warnings.push_back("The settings in " + pathname + " could not be read ("
                           + e.what() + "); the defaults are used.");
      }
      return settings;
    }

    // Writes the user's settings. Throws std::runtime_error, with
    // words fit for the user, if that cannot be done. The file is
    // written beside the old one and then put in its place, so a
    // failure halfway does not leave half a file.
    static void save(const Settings& settings
                     , const std::string& pathname = Paths::config_file()) {
      namespace fs = std::filesystem;
      const fs::path target(pathname);
      if (target.has_parent_path() && !Paths::make_folder(target.parent_path().string()))
        throw std::runtime_error("Could not make the folder " + target.parent_path().string());

      const fs::path fresh = target.string() + ".new";
      {
        std::ofstream file(fresh, std::ios::trunc | std::ios::binary);
        file << to_yaml(settings);
        file.close();
        if (!file)
          throw std::runtime_error("Could not write " + fresh.string());
      }
      std::error_code ec;
      fs::rename(fresh, target, ec);
      if (ec) {
        fs::remove(fresh, ec);
        throw std::runtime_error("Could not write " + pathname);
      }
    }
  };
}
