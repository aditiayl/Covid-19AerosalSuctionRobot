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


    if (!face_cascade.load("haarcascade_frontalface_default.xml"))
    {
        cout << "--(!)Error loading face cascade\n";
        return -1;
    };
    if (!mouth_cascade.load("mouth_aditia3.xml"))
    {
        cout << "--(!)Error loading mouth cascade\n";
        return -1;
    };

    vector<cv::String> fn;
    glob("D:/Robot/Project/Dataset/test/result", fn, false);

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
        if (waitKey(10) == 27)
        {
            break; // escape
        }
        //imwrite(fn[i],frame );
    }
    return 0;
}

void detectAndDisplay(Mat frame)
{   
    int errorX;
    int errorY;

    Mat frame_gray;
    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);

    //center of frame
    Point centerFrame(frame.cols / 2, frame.rows / 2);
    circle(frame, centerFrame, 2, Scalar(0, 0, 255), 4);

    //-- Detect faces
    std::vector<Rect> faces;
    face_cascade.detectMultiScale(frame_gray, faces, 1.1, 3, 0, Size(100, 100));
    for (size_t i = 0; i < faces.size(); i++)
    {
        Point center(faces[i].x + faces[i].width / 2, faces[i].y + faces[i].height / 2);
        ellipse(frame, center, Size(faces[i].width / 2, faces[i].height / 2), 0, 0, 360, Scalar(0, 255, 0), 3);
        Mat faceROI = frame_gray(faces[i]);

        //Making
        cv::Mat mask = cv::Mat::zeros(faceROI.size(), faceROI.type());
        cv::Mat dstImage = cv::Mat::zeros(faceROI.size(), faceROI.type());
        cv::Point P1(0, mask.rows * 0.55);
        cv::Point P2(mask.rows, mask.cols);
        cv::rectangle(mask, P1, P2, cv::Scalar(255, 0, 0), FILLED);
        faceROI.copyTo(dstImage, mask);
        imshow("masking", dstImage);

        //size of frame of face detection
        //printf("x = %d,\t", faces[i].width);
        //printf("y = %d,\n", faces[i].height);

        //-- In each face, detect mouth
        std::vector<Rect> mouth;
        mouth_cascade.detectMultiScale(dstImage, mouth, 1.1, 3, 0, Size(40, 40));
        for (size_t j = 0; j < mouth.size(); j++)
        {
            Point mouth_center(faces[i].x + mouth[j].x + mouth[j].width / 2, faces[i].y + mouth[j].y + mouth[j].height / 2);
            int radius = cvRound((mouth[j].width + mouth[j].height) * 0.25);
            circle(frame, mouth_center, radius, Scalar(255, 0, 0), 3);

            //printf("Point x = %d\t",mouth[i].width);
            //printf("Point y = %d\n", mouth[i].height);

            //Error mouth center based on frame center
            errorX = (frame.cols / 2) - mouth_center.x;
            errorY = (frame.rows / 2) - mouth_center.y;

            //printf("Error x = %d\t", errorX);
            //printf("Error y = %d\n", errorY);
        }
    }
    //-- Show what you got
    imshow("Capture - Face detection", frame);
    waitKey(300);
}



