// Floating.h -- dialogs that float under a tiling window manager.
//
// A tiling window manager (Hyprland, Sway and the like) gives every
// new window a tile of its own, unless the window says it belongs
// to another one. A dialog that says so is floated over its parent
// instead, which is what a dialog should do. Saying so is called
// being "transient for" the parent, and it is the one thing that
// works across compositors with no per-user configuration.
//
// So every dialog of ours is made transient for the main window.
#pragma once

namespace Gui {
  // the application's main window
  inline std::shared_ptr<Gtk::Window> obtain_main_window() {
    return obtain_builder()->get_object<Gtk::Window>(ss::main_window);
  }

  // Makes a dialog float over the main window.
  inline void float_over_main_window(Gtk::Window& dialog) {
    if (auto main_window = obtain_main_window(); main_window && main_window.get() != &dialog)
      dialog.set_transient_for(*main_window);
  }

  // The same for the file dialogs, which are not windows of ours
  // but the system's: they are told which window they belong to,
  // and the system's dialog is floated over that.
  inline void float_over_main_window(Gtk::NativeDialog& dialog) {
    if (auto main_window = obtain_main_window())
      dialog.set_transient_for(*main_window);
  }
}
