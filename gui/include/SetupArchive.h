// SetupArchive.h -- a setup and its gels in one file, for sharing.
//
// The file is an ordinary zip, under the extension .hal. Anything
// that opens zips opens it, once it is told to or the file is
// renamed.
//
// Export packs a setup into a zip: the setup file as setup.yaml at
// the top, and every gel it uses in a gels/ folder beside it. The
// setup file inside refers to the gels as gels/<name>, so it works
// wherever the zip is unpacked.
//
// Import unpacks such a zip into a new folder of its own and hands
// back the setup file found in it, to be loaded like any other. From
// then on it is an ordinary folder: Save writes back into it, and
// the gels are ordinary files in it.
//
// Camera gels are only a device number, and travel as that.
//
// Zip is used, rather than tar, because every system can open it
// without anything extra. The gels are stored as they are, not
// compressed again: images and videos are compressed already, and
// squeezing them a second time only costs time.
#pragma once

#include <zip.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <stdexcept>

namespace Gui {

  class SetupArchive {
  public:
    // what an exported setup's file name ends in
    static inline const std::string extension = ".hal";

    // the name of the setup file inside an exported zip
    static inline const std::string setup_name = "setup.yaml";
    static inline const std::string gel_folder = "gels";

    // Import will not unpack more than this; a zip that claims more
    // is broken or malicious, and not a setup anyone meant to share.
    static constexpr std::uint64_t largest_import = 16ull << 30; // 16 GiB

    // ---- Export

    // Writes `setup` and its gels to `zip_pathname`. Throws
    // std::runtime_error, with words fit for the user, if it cannot.
    static void export_to(Setup setup, const std::string& zip_pathname) {
      namespace fs = std::filesystem;

      // Every gel file must be there before anything is written.
      std::vector<std::string> missing;
      for (auto* gels : {&setup.left, &setup.right})
        for (const auto& gel : *gels)
          if (gel.kind != GelAsset::Kind::camera && !fs::is_regular_file(gel.path))
            missing.push_back(gel.path);
      if (!missing.empty()) {
        std::string list;
        for (const auto& path : missing)
          list += "\n" + path;
        throw std::runtime_error("These gels could not be found, so nothing was exported:" + list);
      }

      int error_code = 0;
      zip_t* zip = zip_open(zip_pathname.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error_code);
      if (!zip) {
        zip_error_t error;
        zip_error_init_with_code(&error, error_code);
        const std::string why = zip_error_strerror(&error);
        zip_error_fini(&error);
        throw std::runtime_error("Could not create " + zip_pathname + ": " + why);
      }

      try {
        // Each gel file goes in once, under a name of its own. Two
        // different files with the same name get -2, -3 ... added.
        // Names are compared without regard to case, since on macOS
        // and Windows A.png and a.png are the same file.
        std::map<std::string, std::string> entry_of; // gel file -> name in the zip
        std::set<std::string> taken;                 // names in use, lower case
        auto lower = [](std::string text) {
          for (auto& c : text) c = char(std::tolower((unsigned char)c));
          return text;
        };

        for (auto* gels : {&setup.left, &setup.right}) {
          for (auto& gel : *gels) {
            if (gel.kind == GelAsset::Kind::camera)
              continue;
            const std::string source = fs::absolute(gel.path).lexically_normal().string();

            if (!entry_of.count(source)) {
              const fs::path file(source);
              std::string name = file.filename().string();
              for (int n = 2; taken.count(lower(name)); ++n)
                name = file.stem().string() + "-" + std::to_string(n) + file.extension().string();
              taken.insert(lower(name));

              const std::string entry = gel_folder + "/" + name;
              add_file(zip, entry, source);
              entry_of[source] = entry;
            }
            gel.path = entry_of[source]; // relative: inside the zip
          }
        }

        // and the setup file itself, compressed: it is text
        const std::string text = setup.to_yaml({});
        zip_source_t* source = zip_source_buffer(zip, text.data(), text.size(), 0);
        if (!source)
          throw std::runtime_error(zip_strerror(zip));
        if (zip_file_add(zip, setup_name.c_str(), source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
          zip_source_free(source);
          throw std::runtime_error(zip_strerror(zip));
        }

        // This is where the zip is really written, gels and all.
        // `text` must still be alive here: the buffer is only read now.
        if (zip_close(zip) < 0)
          throw std::runtime_error(zip_strerror(zip));
      } catch (const std::runtime_error& e) {
        zip_discard(zip); // leaves no half-written file behind
        throw std::runtime_error("Could not export to " + zip_pathname + ": " + e.what());
      }
    }

    // ---- Import

    // Unpacks a setup zip into a new folder of its own, and returns
    // the full pathname of the setup file in it. The folder is put
    // beside the zip and named after it; if the zip's folder cannot
    // be written to, it goes in `fallback_folder` instead. Throws
    // std::runtime_error, with words fit for the user, if the zip is
    // not one Import will unpack; nothing is left behind then.
    static std::string import_from(const std::string& zip_pathname
                                   , const std::filesystem::path& fallback_folder) {
      namespace fs = std::filesystem;

      int error_code = 0;
      zip_t* zip = zip_open(zip_pathname.c_str(), ZIP_RDONLY, &error_code);
      if (!zip) {
        zip_error_t error;
        zip_error_init_with_code(&error, error_code);
        const std::string why = zip_error_strerror(&error);
        zip_error_fini(&error);
        throw std::runtime_error("Could not open " + zip_pathname + ": " + why);
      }

      fs::path folder; // the one we made, if we got that far
      try {
        // First look at everything, before writing anything.
        struct Entry { zip_uint64_t index; std::string name; std::uint64_t size; };
        std::vector<Entry> files;
        std::uint64_t total = 0;
        std::string setup_entry;

        const zip_int64_t count = zip_get_num_entries(zip, 0);
        for (zip_int64_t i = 0; i < count; ++i) {
          zip_stat_t stat;
          if (zip_stat_index(zip, zip_uint64_t(i), 0, &stat) < 0
              || !(stat.valid & ZIP_STAT_NAME) || !(stat.valid & ZIP_STAT_SIZE))
            throw std::runtime_error("an entry in it cannot be read.");

          const std::string name = stat.name;
          if (name.ends_with("/"))
            continue; // a folder; folders are made as they are needed
          if (!is_safe_name(name))
            throw std::runtime_error("it holds a file named \"" + name
                                     + "\", which would be written outside the folder it is "
                                       "unpacked into. It was not unpacked.");

          total += stat.size;
          if (total > largest_import)
            throw std::runtime_error("it would unpack to more than 16 GiB.");
          files.push_back({zip_uint64_t(i), name, stat.size});

          // the setup file: setup.yaml at the top, or failing that
          // the YAML file nearest the top (as when someone zipped a
          // whole setup folder by hand)
          if (name.ends_with(".yaml") || name.ends_with(".yml")) {
            const auto depth = [](const std::string& n) { return std::count(n.begin(), n.end(), '/'); };
            if (setup_entry.empty()
                || name == setup_name
                || (setup_entry != setup_name && depth(name) < depth(setup_entry)))
              setup_entry = name;
          }
        }
        if (setup_entry.empty())
          throw std::runtime_error("there is no setup file (.yaml) in it.");

        folder = new_folder_for(zip_pathname, fallback_folder);

        for (const auto& file : files)
          extract(zip, file.index, folder / fs::path(file.name));

        zip_discard(zip); // opened for reading only; nothing to write
        return (folder / fs::path(setup_entry)).lexically_normal().string();

      } catch (const std::exception& e) {
        zip_discard(zip);
        std::error_code ignore;
        if (!folder.empty())
          fs::remove_all(folder, ignore); // ours, and only half filled
        throw std::runtime_error("Could not import " + zip_pathname + ": " + e.what());
      }
    }

  private:
    // Puts one file into the zip, stored as it is.
    static void add_file(zip_t* zip, const std::string& entry, const std::string& source_path) {
      // 0 for the length means the whole file; it is read when the
      // zip is closed, not now.
      zip_source_t* source = zip_source_file(zip, source_path.c_str(), 0, 0);
      if (!source)
        throw std::runtime_error(source_path + ": " + zip_strerror(zip));

      const zip_int64_t index = zip_file_add(zip, entry.c_str(), source
                                             , ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
      if (index < 0) {
        zip_source_free(source);
        throw std::runtime_error(source_path + ": " + zip_strerror(zip));
      }
      zip_set_file_compression(zip, zip_uint64_t(index), ZIP_CM_STORE, 0);
    }

    // Is this a name that stays inside the folder it is unpacked
    // into? Not if it starts at the root or a drive, climbs out
    // with "..", or uses backslashes, which Windows would read as
    // folder separators.
    static bool is_safe_name(const std::string& name) {
      if (name.empty() || name.front() == '/' || name.find('\\') != std::string::npos)
        return false;
      if (name.size() >= 2 && name[1] == ':')
        return false; // C:...
      std::size_t start = 0;
      while (start <= name.size()) {
        const auto end = name.find('/', start);
        const auto part = name.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (part == "..")
          return false;
        if (end == std::string::npos)
          break;
        start = end + 1;
      }
      return true;
    }

    // A new, empty folder to unpack into, named after the zip:
    // "Sunset" for Sunset.zip, or "Sunset-2" if there is one
    // already. Beside the zip if possible, else in `fallback`.
    static std::filesystem::path new_folder_for(const std::string& zip_pathname
                                                , const std::filesystem::path& fallback) {
      namespace fs = std::filesystem;
      const fs::path zip_file = fs::absolute(zip_pathname);
      const std::string stem = zip_file.stem().string();

      for (const fs::path& parent : {zip_file.parent_path(), fallback}) {
        if (parent.empty())
          continue;
        std::error_code error;
        fs::create_directories(parent, error);
        for (int n = 1; n < 1000; ++n) {
          const fs::path candidate = parent / (n == 1 ? stem : stem + "-" + std::to_string(n));
          if (fs::exists(candidate))
            continue;
          if (fs::create_directory(candidate, error))
            return candidate;
          break; // not allowed to make folders here: try the fallback
        }
      }
      throw std::runtime_error("there is nowhere to unpack it.");
    }

    // Writes one entry of the zip to `target`, making its folders.
    static void extract(zip_t* zip, zip_uint64_t index, const std::filesystem::path& target) {
      std::filesystem::create_directories(target.parent_path());

      zip_file_t* in = zip_fopen_index(zip, index, 0);
      if (!in)
        throw std::runtime_error(target.filename().string() + ": " + zip_strerror(zip));

      std::ofstream out(target, std::ios::binary | std::ios::trunc);
      std::vector<char> buffer(1 << 20);
      bool ok = bool(out);
      while (ok) {
        const zip_int64_t n = zip_fread(in, buffer.data(), buffer.size());
        if (n < 0) { ok = false; break; }
        if (n == 0) break;
        out.write(buffer.data(), std::streamsize(n));
        ok = bool(out);
      }
      zip_fclose(in);
      out.close();
      if (!ok || !out)
        throw std::runtime_error("could not unpack " + target.filename().string()
                                 + " (is the disk full?)");
    }
  };
}
