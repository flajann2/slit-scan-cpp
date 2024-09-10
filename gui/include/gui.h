// gui.h of SlitScan
#pragma once

#include <gtkmm.h>
#include <glibmm/main.h>

#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include <gtkmm/drawingarea.h>
#include <gdkmm/contentformats.h>

#include <cairomm/context.h>

#include <chrono>
#include <ratio>
#include <thread>
#include <iostream>

namespace Gui {
class Window : public Gtk::Window {};
class Pane : public Gtk::DrawingArea {};
}

// in project reference must appear at the end

#include <AppWindow.h>
#include <menu.h>
