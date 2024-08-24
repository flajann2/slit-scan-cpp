// Video Test for OpenCV
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

  string window_name = "My Camera Feed";
  namedWindow(window_name); //create a window called "My Camera Feed"

  const int blurs[] = {3,5,7,9,11,13,15,17,15,13,11,9,7,5,3};
  const int bs = sizeof(blurs) / sizeof(int);
  const int skip = 5; // frames to skip
  int iblur = 0; // index to the blur to use
  int fr = 0;

  cout << "number of blurs is " << bs << endl;
  
  while (true) {
    fr++;
    if (fr % skip == 0) {
      iblur = ++iblur % bs;
    }
      
    Mat frame;
    bool bSuccess = cap.read(frame); // read a new frame from video 

    //Breaking the while loop if the frames cannot be captured
    if (bSuccess == false)  {
      cout << "Video camera is disconnected" << endl;
      cin.get(); //Wait for any key press
      break;
    }
    
    Mat blurredFrame;
    GaussianBlur(frame, blurredFrame, cv::Size(blurs[iblur], blurs[iblur]), 0);
    
    //show the frame in the created window
    imshow(window_name, blurredFrame);

    //wait for for 10 ms until any key is pressed.  
    //If the 'Esc' key is pressed, break the while loop.
    //If the any other key is pressed, continue the loop 
    //If any key is not pressed withing 10 ms, continue the loop 
    if (waitKey(10) == 27)  {
      cout << "Esc key is pressed by user. Stoppig the video" << endl;
      break;
    }
  }

  return 0;
}
