// Frame2Canvas.h -- Blast a cv::Mat frame to a Gtk::DrawImage
#pragma once

#include <gui.h>

namespace Gel {
  // we use this simple struct to keep the parameter count
  // small and uncluttered, and if we need to add
  // additional attributes, it will be a no-brainer.
  struct F2CC {
    int width;
    int height;
    bool resize;          // scale the frame to the canvas
    bool preserve_aspect; // when resizing, letterbox instead of stretching
  };

  // note that this must only be called from a handler to
  // the DrawingArte::set_draw_func(), which entails that
  // queue_draw() or queue_draw_area() must be called
  // every tick.
  //
  // The frame must be 8-bit, 3-channel RGB.
  //
  // void on_draw(const Cairo::RefPtr<Cairo::Context>& cr
  //              , int width
  //              , int height)
  inline void frame_to_canvas(const cv::Mat& frame
                              , const Cairo::RefPtr<Cairo::Context>& cr
                              , const F2CC& cc) {
    if (frame.empty() || cc.width <= 0 || cc.height <= 0)
      return;

    // work out the size and position on the canvas
    int out_w = frame.cols;
    int out_h = frame.rows;
    int off_x = 0;
    int off_y = 0;

    if (cc.resize) {
      out_w = cc.width;
      out_h = cc.height;
      if (cc.preserve_aspect) {
        const double scale = std::min(double(cc.width)  / frame.cols
                                      , double(cc.height) / frame.rows);
        out_w = std::max(1, int(frame.cols * scale));
        out_h = std::max(1, int(frame.rows * scale));
        off_x = (cc.width  - out_w) / 2;
        off_y = (cc.height - out_h) / 2;
      }
    }

    // resized only holds pixels if we really had to resize;
    // otherwise we paint straight from the caller's frame.
    cv::Mat resized;
    if (out_w != frame.cols || out_h != frame.rows) {
      // shrinking has to average over the pixels it drops, or
      // fine detail turns into jaggies and shimmer.
      const bool shrinking = out_w < frame.cols || out_h < frame.rows;
      cv::resize(frame, resized
                 , cv::Size(out_w, out_h)
                 , 0, 0
                 , shrinking ? cv::INTER_AREA : cv::INTER_LINEAR);
    }
    const cv::Mat& out_frame = resized.empty() ? frame : resized;

    // Create a GdkPixbuf from the OpenCV Mat. The pixbuf does not
    // own the pixels, so out_frame must outlive the paint() below.
    Glib::RefPtr<Gdk::Pixbuf> pixbuf =
      Gdk::Pixbuf::create_from_data(out_frame.data
                                    , Gdk::Colorspace::RGB
                                    , false
                                    , 8
                                    , out_frame.cols
                                    , out_frame.rows
                                    , out_frame.step);

    Gdk::Cairo::set_source_pixbuf(cr, pixbuf, off_x, off_y);
    cr->paint();
  }

  inline void rotate(const cv::Mat& frame, cv::Mat& rotated, double angle) {
    cv::Point2f center(frame.cols / 2.0, frame.rows / 2.0);
    cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, angle, 1.0);
    cv::warpAffine(frame, rotated, rotationMatrix, frame.size());
  }
}
