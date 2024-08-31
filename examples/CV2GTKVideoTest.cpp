// CV2GTKVideoTest -- since we will be doing this a lot

#include <opencv2/opencv.hpp>
#include <gtkmm.h>
#include <gdkmm/pixbuf.h>
#include <iostream>
#include <memory>

using namespace std;

class VideoCanvas : public Gtk::DrawingArea {
public:
  VideoCanvas() : width_(800)
                , height_(448)  {

 
        // Create an Image widget to display the video
        image_.set_size_request(800, 448);
        show();

        // Start the video capture
        capture_.open(0); // Open the default camera
        if (!capture_.isOpened()) {
            throw std::runtime_error("Unable to open camera");
        }

        //// // Start the frame update loop
        //// Glib::signal_idle().connect(sigc::mem_fun(*this,
        ////                                           &VideoWindow::on_draw),
        ////                             false);
    }
  ~VideoCanvas() {}
  
  void size_allocate (Gtk::Allocation& allocation, int baseline = -1) {
    // Call the parent to do whatever needs to be done:
    DrawingArea::size_allocate(allocation, baseline);
	
    // Remember the new allocated size for resizing operation:
    width_ = allocation.get_width();
    height_ = allocation.get_height();
  }

  // This is not a normal slot.
  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
    cv::Mat frame;
    capture_ >> frame; // Capture a new frame

    if (!frame.empty()) {
      // Convert the frame from BGR to RGB
      cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
      
      // Create a GdkPixbuf from the OpenCV Mat
      Glib::RefPtr<Gdk::Pixbuf> pixbuf =
        Gdk::Pixbuf::create_from_data(frame.data,
                                      Gdk::Colorspace::RGB,
                                      false,
                                      8,
                                      frame.cols,
                                      frame.rows,
                                      frame.step);

      //cout << "frane.data " << frame.data << endl;
      cout << "frame.step " << frame.step << endl;
      cout << "frame.step[1] " << frame.step[1] << endl;
      /// // Update the image widget
      image_.set(pixbuf);

      // Display
      Gdk::Cairo::set_source_pixbuf(cr, pixbuf);
      cr->paint();
    }
    return true; // Keep the loop running
  }
  
protected:
  int width_;
  int height_;
  
private:
  Gtk::Image image_;
  cv::VideoCapture capture_;
};

class VideoWindow : public Gtk::Window {
public:
  VideoWindow(VideoCanvas& vc) : vc_(vc)
                               , vbox_(Gtk::Orientation::VERTICAL) {
    set_title("Video Stream");
    // Create a vertical box to hold the image
    set_child(vbox_); // Add the box to the window
  }

protected:
  VideoCanvas& vc_;
  Gtk::Box  vbox_;
};

class VideoApp : public Gtk::Application {
public:
  VideoApp(std::string appname = "de.atomlogik.samplevideo")
    : Gtk::Application(appname)
    , window_(vc_) {}

protected:
  VideoCanvas vc_;
  VideoWindow window_;
  
  // Override the startup method to create the window and ball
  virtual void on_activate() override {
    window_.set_title("Video App");
    window_.show();
    this->add_window(window_);
  }
};

int main(int argc, char *argv[]) {
  VideoApp app; //::create("org.gtkmm.example");
  try {
    app.run(argc, argv);
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
