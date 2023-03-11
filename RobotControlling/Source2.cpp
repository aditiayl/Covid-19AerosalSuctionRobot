#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

void detectAndDisplay(Mat frame);
CascadeClassifier mouth_cascade;

int main()
{

    if (!mouth_cascade.load("mouth_aditia3.xml"))
    {
        cout << "--(!)Error loading mouth cascade\n";
        return -1;
    };

    VideoCapture capture;
    capture.open(0);
    if (!capture.isOpened())
    {
        cout << "--(!)Error opening video capture\n";
        return -1;
    }


    Mat frame;
    while (capture.read(frame))
    {
        if (frame.empty())
        {
            cout << "--(!) No captured frame -- Break!\n";
            break;
        }
        //-- 3. Apply the classifier to the frame
        detectAndDisplay(frame);
        
        if (waitKey(10) == 27)
        {
            break; // escape
        }
    }
    return 0;
}
void detectAndDisplay(Mat frame)
{
    int errorX;
    int errorY;
    
    Mat frame_gray;
    //Right  
    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);
    
    //center of frame
    Point centerFrame(frame.cols / 2, frame.rows / 2);
    circle(frame, centerFrame, 2, Scalar(0, 0, 255), 4);
    
    //-- Detect mouth
    std::vector<Rect> mouth;

    // Making
    cv::Mat mask = cv::Mat::zeros(frame_gray.size(), frame_gray.type());
    cv::Mat dstImage = cv::Mat::zeros(frame_gray.size(), frame_gray.type());
    cv::Point P1(0, mask.rows * 0.3);
    cv::Point P2(mask.cols, mask.rows);
    cv::rectangle(mask, P1, P2, cv::Scalar(255, 0, 0), FILLED);
    frame_gray.copyTo(dstImage, mask);
    //imshow("masking", dstImage);
    
    
    mouth_cascade.detectMultiScale(dstImage, mouth, 1.5, 3, 0, Size(150, 150), Size(250,250));

    

    for (size_t i = 0; i < mouth.size(); i++)
    {
        Point mouth_center(mouth[i].x + mouth[i].width / 2, mouth[i].y + mouth[i].height / 2);
        ellipse(frame, mouth_center, Size(mouth[i].width / 2, mouth[i].height / 2), 0, 0, 360, Scalar(255, 0, 255), 4);
        Mat mouthROI = dstImage(mouth[i]);
        imshow("mulut", mouthROI);

        //Point face_center(mouth[i].x + mouth[i].width / 2, mouth[i].y + mouth[i].height *0.1);
        //ellipse(frame, face_center, Size(mouth[i].width, mouth[i].height), 0, 0, 360, Scalar(0, 0, 255), 4);
        
    }
    flip(frame,frame, 1);
    //-- Show what you got
    imshow("Capture - Face detection", frame);
}
