// gui.h of SlitScan
#pragma once

// The following are external header dependencies SS depends on.
#include <opencv2/opencv.hpp>
#include <gtkmm.h>
#include <glibmm/main.h>

#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/drawingarea.h>
#include <gdkmm/contentformats.h>

#include <cairomm/context.h>

#include <yaml-cpp/yaml.h>

#include <chrono>
#include <ratio>
#include <thread>
#include <iostream>

#include <format>
#include <string>
#include <string_view>
#include <charconv>
#include <stdexcept>
#include <vector>
#include <tuple>

namespace Gui {

  class Composition {
  public:
    virtual void show(){};
    virtual void hide(){};
  };
  
  class Window : public Gtk::Window {};
  class Pane : public Gtk::DrawingArea {};
}

// The project includes must appear at the end.
// The order is critical, since header dependencies are
// in this file, and they also are down dependencies they
// have with each other. The idea here is to avoid circular
// dependencies.
#include <Builder.h>
#include <WidgetNames.h>
#include <EntryEditor.h>
#include <VideoConfig.h>
#include <MainSettings.h>
#include <GelConfig.h>
#include <GelAsset.h>
#include <GelSelection.h>
#include <AppWindow.h>
#include <Application.h>
