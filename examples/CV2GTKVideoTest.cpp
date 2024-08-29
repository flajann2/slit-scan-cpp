// CV2GTKVideoTest -- since we will be doing this a lot

#include <opencv2/opencv.hpp>
#include <gtkmm.h>
#include <gdkmm/pixbuf.h>
#include <iostream>
#include <memory>

using namespace std;

class VideoWindow : public Gtk::Window {
public:
  VideoWindow() : vbox(Gtk::Orientation::VERTICAL) {
        set_title("Video Stream");
        set_default_size(800, 448);

        // Create a vertical box to hold the image
        set_child(vbox); // Add the box to the window

        // Create an Image widget to display the video
        image.set_size_request(800, 448);
        vbox.append(image); // Add the image to the box

        show();

        // Start the video capture
        capture.open(0); // Open the default camera
        if (!capture.isOpened()) {
            throw std::runtime_error("Unable to open camera");
        }

        // Start the frame update loop
        Glib::signal_idle().connect(sigc::mem_fun(*this,
                                                  &VideoWindow::on_draw),
                                    false);
    }

  bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override {
        cv::Mat frame;
        capture >> frame; // Capture a new frame

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
            image.set(pixbuf);
        }
        return true; // Keep the loop running
    }

private:
  Gtk::Image image;
  Gtk::Box vbox;
  cv::VideoCapture capture;
};

class VideoApp : public Gtk::Application {
public:
  VideoApp(std::string appname = "de.atomlogik.samplevideo")
    : Gtk::Application(appname) {}

protected:
  VideoWindow window;
  
  // Override the startup method to create the window and ball
  void on_activate() override {
    window.set_title("Video App");
    window.show();
    this->add_window(window);
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
