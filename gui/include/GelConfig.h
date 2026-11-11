// GelConfig.h of SlitScan -- configure individual gels
// for slitscanning.
#pragma once

#include <gels.h>

using namespace std;

namespace Gui {
  // The dialog for one gel: how it is rotated, how fast it spins,
  // and whether it is flipped. What is set here goes straight to
  // the gel's source, which applies it to the frames it sends to
  // the scanner.
  //
  // While the dialog is open it shows the gel as the scanner gets
  // it: turning at the rate set, and live if the gel is a video or
  // a camera. The angle is the source's own, so the dialog and the
  // scan turn together.
  class GelConfig : public Composition {
  public:
    // The Rate slider is in revolutions per `rate_minutes` minutes.
    // At 1.0 that is plain RPM. Make it 2.0 and the same slider
    // setting turns the gel half as fast (revolutions per two
    // minutes); make it 0.5 and it turns twice as fast.
    static constexpr double rate_minutes = 5.0;

    // a slider setting as revolutions per second, which is what
    // the gel sources work in.
    static constexpr double revolutions_per_second(double slider_rate) {
      return slider_rate / (rate_minutes * 60.0);
    }

    // the preview is turned many times a second, so it is done on
    // a copy no bigger than this on its longer side.
    static constexpr int preview_size = 800;

    // `gel_` is a still picture of the gel, shown when the source
    // has no frame of its own to show.
    GelConfig(const cv::Mat& gel_
              , const std::string& pathname_
              , Gel::GelSource& source_)
      : gel(gel_)
      , pathname(pathname_)
      , source(source_) {
      
      // our own local instance. 
      builder = obtain_new_builder();

      // acquire widgets
      gel_config = builder->get_object<Gtk::Dialog>(ss::gel_config);

      gel_note = builder->get_object<Gtk::Notebook>(ss::gel_config_note);
      gel_p1_rotate = builder->get_object<Gtk::Scale>(ss::gel_config_p1_rotate);
      gel_p1_rate   = builder->get_object<Gtk::Scale>(ss::gel_config_p1_rate);
      gel_p1_flip   = builder->get_object<Gtk::Switch>(ss::gel_config_p1_flip);
      gel_p1_canvas = builder->get_object<Gtk::DrawingArea>(ss::gel_config_p1_canvas);
      gel_p2_canvas = builder->get_object<Gtk::DrawingArea>(ss::gel_config_p2_canvas);
      gel_p3_canvas = builder->get_object<Gtk::DrawingArea>(ss::gel_config_p3_canvas);

      cout << "GelConfig widget: " << gel_config << endl;
      
      // Signals and slots
      gel_config->signal_close_request().connect(sigc::mem_fun(*this, &GelConfig::on_close), false);
      gel_config->signal_map().connect(sigc::mem_fun(*this, &GelConfig::on_about_to_open));
      gel_config->signal_unmap().connect(sigc::mem_fun(*this, &GelConfig::on_closed));
      gel_p1_rotate->signal_value_changed().connect(sigc::mem_fun(*this, &GelConfig::on_rotate_change));
      gel_p1_rate->signal_value_changed().connect(sigc::mem_fun(*this, &GelConfig::on_rate_change));
      gel_p1_flip->signal_state_set().connect(sigc::mem_fun(*this, &GelConfig::on_switch), false);

      // Both sliders run from minus to plus with nought in the
      // middle, which is where they start: left of centre turns
      // the gel clockwise, right of centre anticlockwise. The
      // ranges themselves are set in the UI file. A mark shows
      // where the middle is, and the trough is not filled in from
      // the left, since the left is no longer where nothing is.
      for (auto& slider : {gel_p1_rotate, gel_p1_rate}) {
        slider->set_has_origin(false);
        slider->add_mark(0.0, Gtk::PositionType::BOTTOM, "0");
        slider->set_value(0.0);
      }

      // Canvas setup. The controls are on the first page; the
      // other two pages show the same picture with the whole page
      // to itself.
      for (auto& canvas : {gel_p1_canvas, gel_p2_canvas, gel_p3_canvas})
        canvas->set_draw_func(sigc::mem_fun(*this, &GelConfig::on_draw));
    }

    static auto create(const cv::Mat& gel
                       , const std::string& pathname_
                       , Gel::GelSource& source_) -> unique_ptr<GelConfig> {
      return make_unique<GelConfig>(gel, pathname_, source_);
    }

    void show() { gel_config->show(); }
    void hide() { gel_config->hide(); }

    // which page the dialog opens on: 0 image, 1 camera, 2 video
    void set_page(int page) { gel_note->set_current_page(page); }

    // the transformations as currently set in the dialog
    double get_angle() const { return angle; } // degrees
    double get_rate() const { return rate; }   // revolutions per rate_minutes
    bool get_flip() const { return flip; }

    // emitted whenever one of the above changes
    sigc::signal<void()>& signal_changed() { return m_sig_changed; }

  protected:
    // The dialog is on screen: keep the picture moving.
    void on_about_to_open() {
      cout << "Dialog for" << pathname << endl;
      source.set_previewing(true);
      if (!animation.connected())
        animation = Glib::signal_timeout().connect(sigc::mem_fun(*this, &GelConfig::on_animate)
                                                   , 33);
    }

    // The dialog is off screen, however that came about.
    void on_closed() {
      animation.disconnect();
      source.set_previewing(false);
    }

    bool on_close() {
      cout << "GelConfig close event" << endl;
      hide();
      return true;
    }

    // about 30 times a second while the dialog is open
    bool on_animate() {
      // a still gel that is not spinning looks the same as last time
      if (source.is_spinning() || source.is_moving())
        redraw();
      return true;
    }

    void redraw() {
      for (auto& canvas : {gel_p1_canvas, gel_p2_canvas, gel_p3_canvas})
        canvas->queue_draw();
    }

    // The source is told of every change at once, and applies it
    // to the scan as well as to our picture.
    void apply() {
      source.set_transform(angle, revolutions_per_second(rate), flip);
      redraw();
      m_sig_changed.emit();
    }
    
    void on_rotate_change() {
      angle = gel_p1_rotate->get_value();
      apply();
      std::cout << "Rotate: " << angle << std::endl;
    }

    void on_rate_change() {
      rate = gel_p1_rate->get_value();
      apply();
      std::cout << "Rate: " << rate << " revolutions per "
                << rate_minutes << " min" << std::endl;
    }

    bool on_switch(bool state) {
      flip = state;
      apply();
      cout << "Switch: " << (state ? "ON" : "OFF") << " for " << pathname << endl;
      return true; // Return true to allow the state change
    }

    // The gel at preview size. A still image is only scaled once;
    // a frame from a video or a camera is new every time.
    const cv::Mat& preview_of(const cv::Mat& frame) {
      if (frame.data == preview_from && !preview.empty())
        return preview;

      const int longer = std::max(frame.cols, frame.rows);
      if (longer > preview_size) {
        const double scale = double(preview_size) / longer;
        cv::resize(frame, preview, cv::Size(), scale, scale, cv::INTER_AREA);
      } else {
        preview = frame;
      }
      preview_keep = frame; // so that its address stays its own
      preview_from = frame.data;
      return preview;
    }
    
    void on_draw(const Cairo::RefPtr<Cairo::Context>& cr
                 , int width
                 , int height) {
      // what the source has now, or the still picture if it has nothing
      cv::Mat frame = source.preview_frame();
      if (frame.empty())
        frame = gel;
      if (frame.empty())
        return;

      // turned and flipped exactly as the scanner will get it.
      // The proportions are kept: a turning picture that is also
      // stretched to fit wobbles.
      const cv::Mat shown = source.transformed(preview_of(frame), Gel::time_basis::now());
      Gel::frame_to_canvas(shown, cr, Gel::F2CC{width, height, true, true});
    }
    
  private:
    shared_ptr<Gtk::Builder> builder; // our own private instance
    sigc::signal<void()> m_sig_changed;

    const cv::Mat& gel;
    const std::string& pathname;
    Gel::GelSource& source;

    double angle = 0.0; // degrees; positive is anticlockwise
    double rate = 0.0;  // revolutions per rate_minutes; the sign is the direction
    bool flip = false;

    sigc::connection animation; // the timer, while the dialog is open
    cv::Mat preview;            // the gel at preview size
    cv::Mat preview_keep;       // the frame the preview was made from
    const uchar* preview_from = nullptr;
    
    shared_ptr<Gtk::Dialog> gel_config;
    shared_ptr<Gtk::DrawingArea> gel_p1_canvas;
    shared_ptr<Gtk::DrawingArea> gel_p2_canvas;
    shared_ptr<Gtk::DrawingArea> gel_p3_canvas;
    shared_ptr<Gtk::Scale> gel_p1_rotate;
    shared_ptr<Gtk::Scale> gel_p1_rate;
    shared_ptr<Gtk::Switch> gel_p1_flip;
    shared_ptr<Gtk::Notebook> gel_note;

  };
}
