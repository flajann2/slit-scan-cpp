// GelAsset.h of SlitScan -- representing one loaded gel in its 
//                           original format. Transformations shall
//                           be made per instruction.
#pragma once

using namespace std;

const int thumb_width = 400;
const int thumb_height = 300;

namespace Gui {
  
  // This shall display a thumbnail and a dropdown for configuration.
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
      thumbnail.set_pixel_size(50);
      thumbnail.set(pixbuf);      
      prepend(thumbnail);
      append(configuation);
      show();
    }
    
  private:
    Placement place;
    
    Gtk::Image thumbnail;
    cv::Mat mthumb;
    Gtk::Button configuation;
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;
    cv::Mat gel; // source image. All will be derived from this.
    std::string pathname;
  };
}
