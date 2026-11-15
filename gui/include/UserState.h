// UserState.h of SlitScan -- small things remembered from one run to
// the next, such as the folder gels were last loaded from.
//
// They belong to the user and not to a setup, so they are kept apart
// from the setup files: in state.ini in our folder among the user's
// configuration files (~/.config/slitscan; see Paths.h). What the
// user sets in the Settings dialog is kept beside it, in
// config.yaml; see Settings.h.
#pragma once

#include <filesystem>

namespace Gui {
  class UserState {
  public:
    // keys
    static inline const std::string gel_folder = "gel-folder";
    static inline const std::string archive_folder = "archive-folder"; // Import and Export

    // What was remembered under `key`, or "" if nothing was.
    static std::string get(const std::string& key) {
      try {
        auto keyfile = Glib::KeyFile::create();
        keyfile->load_from_file(pathname());
        return keyfile->get_string(group, key);
      } catch (const Glib::Error&) {
        return ""; // no file yet, or no such key
      }
    }

    // Remembers `value` under `key`. Not being able to is no reason
    // to bother the user: it is only a convenience.
    static void set(const std::string& key, const std::string& value) {
      auto keyfile = Glib::KeyFile::create();
      try {
        keyfile->load_from_file(pathname());
      } catch (const Glib::Error&) {
        // no file yet: it starts out empty
      }

      try {
        std::error_code ec;
        std::filesystem::create_directories(std::filesystem::path(pathname()).parent_path(), ec);
        keyfile->set_string(group, key, value);
        keyfile->save_to_file(pathname());
      } catch (const Glib::Error& e) {
        std::cerr << "Could not save " << pathname() << ": " << e.what() << std::endl;
      }
    }

  private:
    static inline const std::string group = "state";

    static std::string pathname() { return Paths::state_file(); }
  };
}
