// Builder.h of SlitScan -- get the all-important builder object
// for slitscanning.
#pragma once

namespace Gui {
using spBuilder = shared_ptr<Gtk::Builder>;
  spBuilder obtain_builder();
}

