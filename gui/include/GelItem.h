// GelItem.h of SlitScan -- representing one loaded gel in its 
//                          original format. Transformations shall
//                          be made per instruction.
#pragma once

using namespace std;

namespace Gui {
  // This shall display a thumbnail and a dropdown for configuration.
  // For instance, rotations, mirroring, or transformations during
  // the gel scan, etc.
  class GelItem : public Gtk::Box {

  public:
    GelItem() {}
    
    static shared_ptr<Gui::GelItem> create_gel_item() {
      auto gel = make_shared<GelItem>();
      return gel;
    }

  protected:
    void setup_gel_item() {
      prepend(thumbnail);
      append(configuation);
      show();
    }
    
  private:
    Gtk::Image thumbnail;
    Gtk::Button configuation;
    
    cv::Mat gel; // source image. All will be derived from this.
    std::string pathname;
  };
}
