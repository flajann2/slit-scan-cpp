// SlitScan Test for OpenCV

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

  const double dWidth = cap.get(CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
  const double dHeight = cap.get(CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

  cout << "Resolution of the video : " << dWidth << " x " << dHeight << endl;

  /*************************************************
   * Define all parameters here                    */
  const int slitWidth = 4; // Width of each slit
  const int slitSpacing = 0; // Space between slits
  const double perspective_factor = 20;
  /*************************************************/
  
  const int slitHeight = dHeight; // Height of the slit (full height of the image)
  const int numberOfSlits = dWidth / (slitWidth + slitSpacing); // Number of slits to extract

  const int sW = (slitWidth + slitSpacing);
  
  // Define source points (corners of the original image)
  cv::Point2f srcPoints[4] = {
    cv::Point2f(0, 0), 
    cv::Point2f(dWidth - 1, 0), 
    cv::Point2f(dWidth - 1, dHeight - 1), 
    cv::Point2f(0, dHeight - 1)
  };
  for (auto pt : srcPoints)
    cout << "srcPoint " << pt << endl;
  cout << endl;
  
  const int higher = dHeight * (perspective_factor - 1.00) / 2.00;
  cout << "higher " << higher << endl;
  
  // Define destination points (where you want those corners to go)
  cv::Point2f dstPoints[4] = {
    cv::Point2f(0, 0), 
    cv::Point2f(dWidth, -higher),
    cv::Point2f(dWidth, dHeight + higher), 
    cv::Point2f(0, dHeight)
  };
  for (auto pt : dstPoints)
    cout << "dstPoint " << pt << endl;
  cout << endl;

  // Create the transformation matrix
  const Mat shift_flat = (Mat_<double>(2, 3) << 1, 0, sW, 0, 1, 0);
  const Mat ss_perspective = cv::getPerspectiveTransform(srcPoints, dstPoints);
  cout << "ss_perspective " << ss_perspective << endl;
  cout << endl;
  

  const string window_name = "Slit Scan 3rd Demo";
  namedWindow(window_name);
  
  Mat frame;
  bool bSuccess = cap.read(frame); // read a new frame from video 

  cv::Mat flatCombined = cv::Mat::zeros(slitHeight,
                                         slitWidth * numberOfSlits +
                                         slitSpacing * (numberOfSlits - 1),
                                         frame.type());
  cv::Mat perspective = cv::Mat::zeros(slitHeight,
                                         slitWidth * numberOfSlits +
                                         slitSpacing * (numberOfSlits - 1),
                                         frame.type());

  int islot = 0;
  while (true) {
    bool bSuccess = cap.read(frame); // read a new frame from video 

    //Breaking the while loop if the frames cannot be captured
    if (bSuccess == false)  {
      cout << "Video camera is disconnected" << endl;
      cin.get(); //Wait for any key press
      break;
    }

    if (islot >= numberOfSlits)
      islot = 0;
    //int x = islot * (slitWidth + slitSpacing); // x position for the slit
    int x = dWidth / 2; // x position for the slit
    Rect slitRegion(x, 0, slitWidth, slitHeight); 
    Mat slit = frame(slitRegion);
    flip(slit, slit, 1);
     
    
    warpAffine(flatCombined, flatCombined, shift_flat, flatCombined.size());
    slit.copyTo(flatCombined(cv::Rect(0, 0, slitWidth, slitHeight)));
    warpPerspective(flatCombined, perspective, ss_perspective, frame.size());
    ++islot;
    
    //show the frame in the created window
    imshow(window_name, perspective);
    //imshow(window_name, flatCombined);
 
    if (waitKey(1) == 27)  {
      cout << "Esc key is pressed by user. Stoppig the video" << endl;
      break;
    }
  }

  return 0;
}
