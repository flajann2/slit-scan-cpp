// CV2GTKVideoTest -- since we will be doing this a lot
// This code demonstrates how to get a OpenCV video stream
// to display in a gtkmm window.

//#include <opencv2/opencv.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include <gtkmm.h>
#include <gdkmm/pixbuf.h>
#include <iostream>
#include <memory>

using namespace std;

class VideoCanvas : public Gtk::DrawingArea {
public:
  VideoCanvas() : width_(800)
                , height_(448)  {

    property_content_width() = width_;
    property_content_height() = height_;
    // Create an Image widget to display the video
    //image_.set_size_request(800, 448);
    show();

    // Start the video capture
    capture_.open(0); // Open the default camera
    if (!capture_.isOpened()) {
      throw std::runtime_error("Unable to open camera");
    }
    set_draw_func(sigc::mem_fun(*this, &VideoCanvas::on_draw));
    Glib::signal_timeout()
      .connect(sigc::bind(sigc::mem_fun(*this, &VideoCanvas::refresh_window)),
               1000/60); // 60 times a second
    }
  ~VideoCanvas() {}
  
  void size_allocate (Gtk::Allocation& allocation, int baseline = -1) {
    // Call the parent to do whatever needs to be done:
    DrawingArea::size_allocate(allocation, baseline);
	
    // Remember the new allocated size for resizing operation:
    width_ = allocation.get_width();
    height_ = allocation.get_height();
  }

  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr
               , int width
               , int height) {
    width_ = width;
    height_ = height;
    cout << "(w,h) = (" << width_ << "," << height_ << ")" << endl;
    
    cv::Mat frame;
    capture_ >> frame; // Capture a new frame

    if (!frame.empty()) {
      // Convert the frame from BGR to RGB
      cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

      // resize to current canvas size
      cv::Mat resized_frame;
      cv::resize(frame, resized_frame
                 , cv::Size(width_, height_)
                 , cv::INTER_LINEAR);      
      
      // Create a GdkPixbuf from the OpenCV Mat
      Glib::RefPtr<Gdk::Pixbuf> pixbuf =
        Gdk::Pixbuf::create_from_data(resized_frame.data,
                                      Gdk::Colorspace::RGB,
                                      false,
                                      8,
                                      resized_frame.cols,
                                      resized_frame.rows,
                                      resized_frame.step);

      // Update the image widget
      //image_.set(pixbuf);

      // Display
      Gdk::Cairo::set_source_pixbuf(cr, pixbuf);
      cr->paint();
    }
    return true; // Keep the loop running
  }
  
  bool refresh_window() {
    queue_draw();
    return true;
  }
  
protected:
  int width_;
  int height_;
  
private:
	sigc::connection update_connection_;
  //Gtk::Image image_;
  cv::VideoCapture capture_;
};

class VideoWindow : public Gtk::Window {
public:
  VideoWindow(VideoCanvas& vc) : vc_(vc) {
    set_title("Video Stream");
    set_child(vc_);
  }

  
protected:
  VideoCanvas& vc_;
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
    cout << "on_activate called." << endl;
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
