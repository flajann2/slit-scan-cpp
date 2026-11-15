// Paths.h of SlitScan -- where things are kept on this system.
//
// This is the one place that knows: the folder for the settings
// file and the remembered state, and the folder recordings go to
// unless another is chosen. Everything else asks here.
//
// On Linux these follow the XDG conventions:
//
//   settings and state   $XDG_CONFIG_HOME/slitscan   (~/.config/slitscan)
//   recordings           the user's Videos folder    (~/Videos)
//
// GLib answers both questions, and has answers for macOS and
// Windows too, but not the ones those systems' own applications
// use (on macOS, for one, it says ~/.config, where a Mac
// application keeps its settings under ~/Library/Application
// Support). When Slit Scan goes to those systems, their logic goes
// into config_folder() and default_video_folder() here, and
// nowhere else.
#pragma once

#include <filesystem>

namespace Gui::Paths {
  // the name of our folder among the other applications'
  inline const std::string app_folder = "slitscan";

  // The folder our own files are kept in. It need not exist yet;
  // see make_folder().
  inline std::string config_folder() {
    return Glib::build_filename(Glib::get_user_config_dir(), app_folder);
  }

  // What is set in the Settings dialog; see Settings.h.
  inline std::string config_file() {
    return Glib::build_filename(config_folder(), "config.yaml");
  }

  // Small things remembered from one run to the next; see UserState.h.
  inline std::string state_file() {
    return Glib::build_filename(config_folder(), "state.ini");
  }

  // Where recordings go unless the user has chosen a folder: the
  // user's Videos folder, or the home folder if there is none.
  inline std::string default_video_folder() {
    const std::string videos = Glib::get_user_special_dir(Glib::UserDirectory::VIDEOS);
    if (!videos.empty() && Glib::file_test(videos, Glib::FileTest::IS_DIR))
      return videos;
    return Glib::get_home_dir();
  }

  // A path as the user may have typed it: "~" and "~/..." stand
  // for the home folder, and blanks around it are dropped.
  inline std::string expanded(std::string path) {
    const auto first = path.find_first_not_of(" \t");
    const auto last = path.find_last_not_of(" \t");
    path = (first == std::string::npos) ? "" : path.substr(first, last - first + 1);
    if (path == "~")
      return Glib::get_home_dir();
    if (path.starts_with("~/"))
      return Glib::build_filename(Glib::get_home_dir(), path.substr(2));
    return path;
  }

  // Makes the folder, and those above it, if it is not there.
  // Returns whether it is there afterwards.
  inline bool make_folder(const std::string& folder) {
    std::error_code ec;
    std::filesystem::create_directories(folder, ec);
    return std::filesystem::is_directory(folder, ec);
  }

  // The folder recordings are to go to, given the one the user
  // wants (empty for the default). It is made if it is not there.
  // If that cannot be done the default is used instead, and `note`
  // says so.
  inline std::string video_folder(const std::string& wanted, std::string& note) {
    const std::string folder = expanded(wanted);
    if (folder.empty())
      return default_video_folder();
    if (make_folder(folder))
      return folder;
    note = "The folder " + folder + " could not be had; recording to "
           + default_video_folder() + " instead.";
    return default_video_folder();
  }
}
