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
    
    GelAsset(Placement place_ = Placement::undefined) : place(place_) {}
    
    static shared_ptr<Gui::GelAsset> create_gel_asset(std::string gel_pathname
                                                      , Placement place_ = Placement::undefined) {
      auto gel = make_shared<GelAsset>();
      gel->setup_gel_item(gel_pathname);
      return gel;
    }

    auto get_placement() { return place; }
    
  protected:
    void setup_gel_item(std::string gel_pathname) {
      pathname = gel_pathname;
      cout << "pix: " << pathname << endl;
      gel = cv::imread(pathname, cv::IMREAD_COLOR);
      cv::cvtColor(gel, gel, cv::COLOR_BGR2RGB);
      cv::resize(gel, mthumb
                 , cv::Size(thumb_width, thumb_height)
                 , cv::INTER_LINEAR);
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

      gel_config = GelConfig::create(gel, pathname);
      configuration.signal_clicked().connect(sigc::mem_fun(*this, &GelAsset::on_config_clicked));
      
      show();
    }

    void on_config_clicked() {
      cout << "Config clicked. file " << pathname << endl;
      gel_config->show();
    }
    
  private:
    Placement place;

    // ImageGelSource for shits and giggles for now.
    Gel::ImageGelSource m_source;
    
    unique_ptr<Gel::GelSource> gel_source;
    unique_ptr<Gui::GelConfig> gel_config;

    Gtk::Image thumbnail;
    cv::Mat mthumb;
    Gtk::Button configuration;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;

    cv::Mat gel; // source image. All will be derived from this.
    std::string pathname;
  };
}
