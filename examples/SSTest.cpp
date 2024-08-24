// SlitScan Test for OpenCV
// Note that this is not my style of coding as I don't
// like using breaks to get out of while loops, but
// this is simply an example.

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char* argv[]) {
  //Open the default video camera
  VideoCapture cap(0);

  // if not success, exit program
  if (cap.isOpened() == false) {
      cout << "Cannot open the video camera" << endl;
      cin.get(); //wait for any key press
      return -1;
    } 

  double dWidth = cap.get(CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
  double dHeight = cap.get(CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

  cout << "Resolution of the video : " << dWidth << " x " << dHeight << endl;

  // Define slit parameters
  const int slitWidth = 10; // Width of each slit
  const int slitHeight = dHeight; // Height of the slit (full height of the image)
  const int numberOfSlits = 20; // Number of slits to extract
  const int slitSpacing = 1; // Space between slits

  const string window_name = "Slit Scan Demo";
  namedWindow(window_name);

  while (true) {
    Mat frame;
    bool bSuccess = cap.read(frame); // read a new frame from video 

    //Breaking the while loop if the frames cannot be captured
    if (bSuccess == false)  {
      cout << "Video camera is disconnected" << endl;
      cin.get(); //Wait for any key press
      break;
    }

    // create the slitscan
    cv::Mat combinedSlits = cv::Mat::zeros(slitHeight,
                                           slitWidth * numberOfSlits +
                                           slitSpacing * (numberOfSlits - 1), frame.type());
    for (int i = 0; i < numberOfSlits; ++i) {
        int x = i * (slitWidth + slitSpacing); // x position for the slit
        cv::Rect slitRegion(i * slitWidth + i * slitSpacing, 0, slitWidth, slitHeight); // Define the slit region
        cv::Mat slit = frame(slitRegion);
        slit.copyTo(combinedSlits(cv::Rect(x, 0, slitWidth, slitHeight)));
    }
    
    //show the frame in the created window
    imshow(window_name, combinedSlits);
 
    if (waitKey(10) == 27)  {
      cout << "Esc key is pressed by user. Stoppig the video" << endl;
      break;
    }
  }

  return 0;
}
