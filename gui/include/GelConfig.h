// GelConfig.h of SlitScan -- configure individual gels
// for slitscanning.
#pragma once

using namespace std;

namespace Gui {
  class GelConfig : public Composition {
  public:
    GelConfig(const cv::Mat& gel_) : gel(gel_) {
      auto builder = obtain_builder();

      // acquire widgets
      gel_config = builder->get_object<Gtk::Dialog>(ss::gel_config);

      gel_note = builder->get_object<Gtk::Notebook>(ss::gel_config_note);
      gel_p1_rotate = builder->get_object<Gtk::Scale>(ss::gel_config_p1_rotate);
      gel_p1_rate   = builder->get_object<Gtk::Scale>(ss::gel_config_p1_rate);
      gel_p1_flip   = builder->get_object<Gtk::Switch>(ss::gel_config_p1_flip);
      gel_p1_canvas = builder->get_object<Gtk::DrawingArea>(ss::gel_config_p1_canvas);

      // Signals and slots
      gel_config->signal_close_request().connect(sigc::mem_fun(*this, &GelConfig::on_close), false);
  }

    static auto create(const cv::Mat& gel) -> unique_ptr<GelConfig> {
      return make_unique<GelConfig>(gel);
    }

    void show() { gel_config->show(); }
    void hide() { gel_config->hide(); }

  protected:
    bool on_close() {
      cout << "close event" << endl;
      gel_config->hide();
      return true;
    }
    
  private:
    const cv::Mat& gel;
    
    shared_ptr<Gtk::Dialog> gel_config;
    shared_ptr<Gtk::DrawingArea> gel_p1_canvas;
    shared_ptr<Gtk::Scale> gel_p1_rotate;
    shared_ptr<Gtk::Scale> gel_p1_rate;
    shared_ptr<Gtk::Switch> gel_p1_flip;
    shared_ptr<Gtk::Notebook> gel_note;

  };
}
