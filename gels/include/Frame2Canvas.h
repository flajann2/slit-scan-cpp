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
    bool resize;
    bool preserve_aspect;
  };

  // note that this must only be called from a handler to
  // the DrawingArte::set_draw_func(), which entails that
  // queue_draw() or queue_draw_area() must be called
  // every tick.
  //
  // void on_draw(const Cairo::RefPtr<Cairo::Context>& cr
  //              , int width
  //              , int height) 
  void frame_to_canvas(const cv::Mat& frame
                       , const Cairo::RefPtr<Cairo::Context>& cr
                       , const F2CC& cc) {
    // resize to current canvas size
    cv::Mat resized_frame;
    cv::Mat& out_frame = resized_frame;

    if (cc.resize) {
      cv::resize(frame, resized_frame
                 , cv::Size(cc.width, cc.height)
                 , cv::INTER_LINEAR);
      out_frame = resized_frame;
    } else {
      out_frame = frame; // TODO: move/copy semantics? reference?
    }

    // Create a GdkPixbuf from the OpenCV Mat
    Glib::RefPtr<Gdk::Pixbuf> pixbuf =
      Gdk::Pixbuf::create_from_data(out_frame.data
                                    , Gdk::Colorspace::RGB
                                    , false
                                    , 8
                                    , resized_frame.cols
                                    , resized_frame.rows
                                    , resized_frame.step);

    Gdk::Cairo::set_source_pixbuf(cr, pixbuf);
    cr->paint();
  }
}
