#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <stdio.h>
#include <dos.h> 

using namespace std;
using namespace cv;

void detectAndDisplay(Mat frame);
CascadeClassifier face_cascade;
CascadeClassifier mouth_cascade;
int main()
{


    if (!face_cascade.load("mouth_aditia3.xml"))
    {
        cout << "--(!)Error loading face cascade\n";
        return -1;
    };
    

    vector<cv::String> fn;
    glob("D:/Robot/Project/Dataset/test", fn, false);

    Mat frame;
    size_t count = fn.size();
    for (rsize_t i = 0; i < count; i++) {
        frame = imread(fn[i]);
        if (frame.empty())
        {
            cout << "--(!) No captured frame -- Break!\n";
            break;
        }
        //-- 3. Apply the classifier to the frame
        detectAndDisplay(frame);
        imwrite(fn[i],frame);
        if (waitKey(10) == 27)
        {
            break; // escape
        }
    }
    return 0;
}

void detectAndDisplay(Mat frame)
{
    Mat frame_gray;
    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);

    //Making
    cv::Mat mask = cv::Mat::zeros(frame_gray.size(), frame_gray.type());
    cv::Mat dstImage = cv::Mat::zeros(frame_gray.size(), frame_gray.type());
    cv::Point P1(0, mask.rows * 0.4);
    cv::Point P2(mask.rows, mask.cols);
    cv::rectangle(mask, P1, P2, cv::Scalar(255, 0, 0), FILLED);
    frame_gray.copyTo(dstImage, mask);
    imshow("masking", dstImage);

    //-- Detect faces
    std::vector<Rect> faces;
    face_cascade.detectMultiScale(dstImage, faces, 2.0, 3, 0, Size(40, 40));
    for (size_t i = 0; i < faces.size(); i++)
    {
        Point center(faces[i].x + faces[i].width / 2, faces[i].y + faces[i].height / 2);
        ellipse(frame, center, Size(faces[i].width / 2, faces[i].height / 2), 0, 0, 360, Scalar(0, 255, 0), 3);
        Mat faceROI = frame_gray(faces[i]);
    }
    //-- Show what you got
    imshow("Capture - Face detection", frame);
    waitKey(500);
}



