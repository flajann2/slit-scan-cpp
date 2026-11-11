// Splash.h -- the splash screen shown when the application starts.
//
// A small window with no frame, floating over the main window. It
// goes away by itself after splash_seconds, or at once on a click
// or a key. Start the application with --no-splash to do without.
#pragma once

#include <cmath>

namespace Gui {

  class Splash : public Gtk::Window {
  public:
    static constexpr double splash_seconds = 3.0; // how long it stays
    static constexpr int width  = 640;
    static constexpr int height = 360;

    Splash() {
      set_title("Slit Scan");
      set_decorated(false);
      set_resizable(false);
      set_default_size(width, height);
      float_over_main_window(*this);

      canvas.set_content_width(width);
      canvas.set_content_height(height);
      canvas.set_draw_func(sigc::mem_fun(*this, &Splash::on_draw));
      set_child(canvas);

      // a click or a key and it is gone
      auto click = Gtk::GestureClick::create();
      click->signal_pressed().connect([this](int, double, double) { dismiss(); });
      add_controller(click);

      auto key = Gtk::EventControllerKey::create();
      key->signal_key_pressed().connect([this](guint, guint, Gdk::ModifierType) {
        dismiss();
        return true;
      }, false);
      add_controller(key);

      // and otherwise after a little while
      timer = Glib::signal_timeout().connect([this] {
        dismiss();
        return false;
      }, int(splash_seconds * 1000));
    }

    ~Splash() override { timer.disconnect(); }

    void dismiss() {
      timer.disconnect();
      hide();
    }

  protected:
    // a colour for a place along the scan, from a slow rainbow
    static void stripe_colour(const Cairo::RefPtr<Cairo::Context>& cr, double t, double light) {
      const double two_pi = 6.283185307179586;
      cr->set_source_rgb(light * (0.5 + 0.5 * std::cos(two_pi * (t + 0.00)))
                         , light * (0.5 + 0.5 * std::cos(two_pi * (t + 0.33)))
                         , light * (0.5 + 0.5 * std::cos(two_pi * (t + 0.67))));
    }

    // The picture: what a slit scan looks like, drawn by hand.
    // Stripes that are thin in the middle and wide at the edges,
    // the same on both sides, with lines fanning out from the
    // centre line as the perspective does.
    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr, int w, int h) {
      const double cx = w / 2.0;
      const double cy = h / 2.0;

      cr->set_source_rgb(0, 0, 0);
      cr->paint();

      // the stripes
      const int stripes = 46;
      auto edge = [&](int i) { return cx * std::pow(double(i) / stripes, 2.4); };
      for (int i = 0; i < stripes; ++i) {
        const double x0 = edge(i);
        const double x1 = edge(i + 1) + 0.5;
        const double light = (i % 3 == 1) ? 0.35 : 0.95;
        stripe_colour(cr, i * 0.045, light);
        cr->rectangle(cx + x0, 0, x1 - x0, h);
        cr->fill();
        cr->rectangle(cx - x1, 0, x1 - x0, h);
        cr->fill();
      }

      // the lines fanning out
      cr->set_source_rgba(0, 0, 0, 0.45);
      cr->set_line_width(1.5);
      for (int i = -6; i <= 6; ++i) {
        const double y_mid = cy + i * h / 60.0;
        const double y_edge = cy + i * h / 7.0;
        for (double side : {-1.0, 1.0}) {
          cr->move_to(cx, y_mid);
          cr->line_to(cx + side * cx, y_edge);
          cr->stroke();
        }
      }

      // a band across the middle for the words
      cr->set_source_rgba(0, 0, 0, 0.62);
      cr->rectangle(0, cy - 62, w, 124);
      cr->fill();

      auto words = [&](const Glib::ustring& text, const char* font, double y, double alpha) {
        auto layout = create_pango_layout(text);
        layout->set_font_description(Pango::FontDescription(font));
        int tw = 0, th = 0;
        layout->get_pixel_size(tw, th);
        cr->set_source_rgba(1, 1, 1, alpha);
        cr->move_to(cx - tw / 2.0, y - th / 2.0);
        layout->show_in_cairo_context(cr);
      };
      words("Slit Scan", "Sans Bold 40", cy - 12, 1.0);
      words("click, or press a key, to continue", "Sans 11", cy + 38, 0.75);
    }

  private:
    Gtk::DrawingArea canvas;
    sigc::connection timer;
  };
}
