// GelAsset.h of SlitScan -- representing one loaded gel in its 
//                           original format. Transformations shall
//                           be made per instruction.
#pragma once

#include <gels.h>
using namespace std;

const int thumb_width = 400;
const int thumb_height = 300;

namespace Gui {
  
  // This shall display a thumbnail and a dialog for configuration.
  // For instance, rotations, mirroring, or transformations during
  // the gel scan, etc.
  //
  // A gel is a still image, a video, or a camera. Each kind has its
  // own kind of source, and the rest of the application only ever
  // deals with the Gel::GelSource they have in common.
  class GelAsset : public Gtk::Box {

  public:
    enum class Placement {
      undefined
      , left
      , right
      , top // TODO do we wish to use this?
      , bottom // same question?
      , circular
    };

    enum class Kind { image, video, camera };
    
    GelAsset(Placement place_ = Placement::undefined) : place(place_) {}

    // A gel from a file: an image if it can be read as one, and
    // otherwise a video. Returns nothing if it is neither.
    static shared_ptr<Gui::GelAsset> create_gel_asset(std::string gel_pathname
                                                      , Placement place_ = Placement::undefined) {
      auto gel = make_shared<GelAsset>(place_);
      if (!gel->setup_from_file(gel_pathname)) {
        cerr << "SlitScan: " << gel_pathname
             << " is neither an image nor a video we can read." << endl;
        return {};
      }
      return gel;
    }

    // A gel from a camera, live.
    static shared_ptr<Gui::GelAsset> create_camera_asset(int index
                                                         , Placement place_ = Placement::undefined) {
      auto gel = make_shared<GelAsset>(place_);
      gel->setup_from_camera(index);
      return gel;
    }

    auto get_placement() { return place; }
    auto get_kind() { return kind; }

    // the source that broadcasts this gel's frames
    Gel::GelSource& source() { return *gel_source; }
    
  protected:
    bool setup_from_file(std::string gel_pathname) {
      pathname = gel_pathname;
      cout << "pix: " << pathname << endl;

      gel = cv::imread(pathname, cv::IMREAD_COLOR);
      if (!gel.empty()) {
        kind = Kind::image;
        cv::cvtColor(gel, gel, cv::COLOR_BGR2RGB);

        auto image_source = make_unique<Gel::ImageGelSource>();
        Gel::ImageGelSource::OneGel one;
        one.frame = gel;
        one.name = pathname;
        one.load_time = Gel::time_basis::now();
        one.meth = (place == Placement::right) ? Gel::ScanMeth::Right
                                               : Gel::ScanMeth::Left;
        image_source->set_gel(one);
        gel_source = std::move(image_source);
      } else {
        // not an image, so perhaps a video. Its first frame
        // stands in for it in the thumbnail and the preview.
        gel = Gel::VideoGelSource::first_frame(pathname);
        if (gel.empty())
          return false;
        kind = Kind::video;
        gel_source = make_unique<Gel::VideoGelSource>(pathname);
      }

      setup_widgets();
      return true;
    }

    void setup_from_camera(int index) {
      kind = Kind::camera;
      pathname = "camera " + std::to_string(index);

      // We do not switch the camera on just to make a thumbnail,
      // so a placeholder stands in for it.
      gel = cv::Mat(thumb_height, thumb_width, CV_8UC3, cv::Scalar(40, 40, 40));
      cv::putText(gel, "CAMERA " + std::to_string(index)
                  , cv::Point(40, thumb_height / 2 + 15)
                  , cv::FONT_HERSHEY_SIMPLEX, 1.6
                  , cv::Scalar(230, 230, 230), 3, cv::LINE_AA);

      gel_source = make_unique<Gel::CameraGelSource>(index);
      setup_widgets();
    }

    void setup_widgets() {
      cv::resize(gel, mthumb
                 , cv::Size(thumb_width, thumb_height)
                 , 0, 0
                 , cv::INTER_AREA);
      pixbuf = Gdk::Pixbuf::create_from_data(mthumb.data,
                                                  Gdk::Colorspace::RGB,
                                                  false,
                                                  8,
                                                  mthumb.cols,
                                                  mthumb.rows,
                                                  mthumb.step);
      thumbnail.set_pixel_size(80);
      thumbnail.set(pixbuf);      
      prepend(thumbnail);
      append(configuration);
      set_tooltip_text(pathname);

      // The configuration dialog works on the source directly:
      // what is set there is what the scanner gets. It opens on
      // the page for this kind of gel.
      gel_config = GelConfig::create(gel, pathname, *gel_source);
      gel_config->set_page(kind == Kind::camera ? 1 : kind == Kind::video ? 2 : 0);
      configuration.signal_clicked().connect(sigc::mem_fun(*this, &GelAsset::on_config_clicked));
      
      show();
    }

    void on_config_clicked() {
      cout << "Config clicked. file " << pathname << endl;
      gel_config->show();
    }
    
  private:
    Placement place;
    Kind kind = Kind::image;

    // in this order: the dialog holds on to the source, so it is
    // declared after it and goes away before it.
    unique_ptr<Gel::GelSource> gel_source;
    unique_ptr<Gui::GelConfig> gel_config;

    Gtk::Image thumbnail;
    cv::Mat mthumb;
    Gtk::Button configuration;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;

    // source image. For an image, all will be derived from this.
    // For a video it is the first frame, and for a camera a
    // placeholder; those two are only for looking at.
    cv::Mat gel;
    std::string pathname;
  };
}
