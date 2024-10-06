// Builder.h of SlitScan -- get the all-important builder object
//                          for slitscanning.
#pragma once

namespace Gui {
  // for creating new instances of Builder
  std::shared_ptr<Gtk::Builder> obtain_new_builder();

  // For retriving system-wide instance of the same builder
  std::shared_ptr<Gtk::Builder> obtain_builder();
}
