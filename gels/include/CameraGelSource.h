// CameraGelSource.h -- GelSource for webcam sources
#pragma once

namespace Gel {
  using namespace std;

  // A camera as a gel, live. `index` is OpenCV's number for the
  // device; 0 is the default camera.
  //
  // The camera is only opened once the source becomes active, so
  // adding one to a list does not switch the camera on.
  class CameraGelSource : public CaptureGelSource {
  public:
    explicit CameraGelSource(int index = 0)
      : CaptureGelSource([index](cv::VideoCapture& cap) { return cap.open(index); }
                         , false
                         , "camera " + std::to_string(index)) {}
  };
}
