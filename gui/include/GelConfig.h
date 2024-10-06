// GelConfig.h of SlitScan -- configure individual gels
// for slitscanning.
#pragma once

#include <gels.h>

using namespace std;

namespace Gui {
  class GelConfig : public Composition {
  public:
    GelConfig(const cv::Mat& gel_, const std::string& pathname_)
      : gel(gel_)
      , pathname(pathname_) {
      
      // our own local instance. 
      builder = obtain_new_builder();

      // acquire widgets
      gel_config = builder->get_object<Gtk::Dialog>(ss::gel_config);

      gel_note = builder->get_object<Gtk::Notebook>(ss::gel_config_note);
      gel_p1_rotate = builder->get_object<Gtk::Scale>(ss::gel_config_p1_rotate);
      gel_p1_rate   = builder->get_object<Gtk::Scale>(ss::gel_config_p1_rate);
      gel_p1_flip   = builder->get_object<Gtk::Switch>(ss::gel_config_p1_flip);
      gel_p1_canvas = builder->get_object<Gtk::DrawingArea>(ss::gel_config_p1_canvas);

      cout << "GelConfig widget: " << gel_config << endl;
      
      // Signals and slots
      gel_config->signal_close_request().connect(sigc::mem_fun(*this, &GelConfig::on_close), false);
      gel_config->signal_map().connect(sigc::mem_fun(*this, &GelConfig::on_about_to_open));
      gel_p1_rotate->signal_value_changed().connect(sigc::mem_fun(*this, &GelConfig::on_rotate_change));
      gel_p1_rate->signal_value_changed().connect(sigc::mem_fun(*this, &GelConfig::on_rate_change));
      gel_p1_flip->signal_state_set().connect(sigc::mem_fun(*this, &GelConfig::on_switch), false);

      // Canvas setup
      gel_p1_canvas->set_draw_func(sigc::mem_fun(*this, &GelConfig::on_draw));
    }

    static auto create(const cv::Mat& gel, const std::string& pathname_) -> unique_ptr<GelConfig> {
      return make_unique<GelConfig>(gel, pathname_);
    }

    void show() { gel_config->show(); }
    void hide() { gel_config->hide(); }

  protected:
    void on_about_to_open() {
      cout << "Dialog for" << pathname << endl;
    }

    bool on_close() {
      cout << "GelConfig close event" << endl;
      hide();
      return true;
    }

    void on_rotate_change() {
      double value = gel_p1_rotate->get_value();
      std::cout << "Rotate: " << value << std::endl;
    }

    void on_rate_change() {
      double value = gel_p1_rate->get_value();
      std::cout << "Rate: " << value << std::endl;
    }

    bool on_switch(bool state) {
      cout << "Switch: " << (state ? "ON" : "OFF") << " for " << pathname << endl;
      return true; // Return true to allow the state change
    }
    
    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr
                 , int width
                 , int height) {
      Gel::frame_to_canvas(gel, cr, Gel::F2CC{width, height, true, false});
    }
    
  private:
    shared_ptr<Gtk::Builder> builder; // our own private instance

    const cv::Mat& gel;
    const std::string& pathname;
    
    shared_ptr<Gtk::Dialog> gel_config;
    shared_ptr<Gtk::DrawingArea> gel_p1_canvas;
    shared_ptr<Gtk::Scale> gel_p1_rotate;
    shared_ptr<Gtk::Scale> gel_p1_rate;
    shared_ptr<Gtk::Switch> gel_p1_flip;
    shared_ptr<Gtk::Notebook> gel_note;

  };
}
