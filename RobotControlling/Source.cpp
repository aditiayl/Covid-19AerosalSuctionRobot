#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

void detectAndDisplay(Mat frame);
CascadeClassifier face_cascade;
CascadeClassifier mouth_cascade;
int main()
{
    //data set haarcascade_frontalface_default.xml
    if (!face_cascade.load("face22.xml"))
    {
        cout << "--(!)Error loading face cascade\n";
        return -1;
    };
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
    circle(frame, centerFrame, 1.1, Scalar(0, 0, 255), 4);

    //-- Detect faces
    std::vector<Rect> faces;
    face_cascade.detectMultiScale(frame_gray, faces, 1.1, 3, 0, Size(100, 100));

    for (size_t i = 0; i < faces.size(); i++)
    {
        Point center(faces[i].x + faces[i].width / 2, faces[i].y + faces[i].height / 2);
        ellipse(frame, center, Size(faces[i].width / 2, faces[i].height / 2), 0, 0, 360, Scalar(0, 255, 0), 4);
        Mat faceROI = frame_gray(faces[i]);

        //Making
        cv::Mat mask = cv::Mat::zeros(faceROI.size(), faceROI.type());
        cv::Mat dstImage = cv::Mat::zeros(faceROI.size(), faceROI.type());
        cv::Point P1(0, mask.rows * 0.55);
        cv::Point P2(mask.rows, mask.cols);
        cv::rectangle(mask, P1, P2, cv::Scalar(255, 0, 0), FILLED);
        faceROI.copyTo(dstImage, mask);
        imshow("masking", dstImage);
        faceROI.size();

        //-- In each face, detect mouth
        std::vector<Rect> mouth;
        mouth_cascade.detectMultiScale(dstImage, mouth, 1.1, 3, 0, Size(80, 80));
        for (size_t j = 0; j < mouth.size(); j++)
        {
            Point mouth_center(faces[i].x + mouth[j].x + mouth[j].width / 2, faces[i].y + mouth[j].y + mouth[j].height / 2);
            int radius = cvRound((mouth[j].width + mouth[j].height) * 0.23);
            circle(frame, mouth_center, radius, Scalar(255, 0, 0), 4);

            //printf("Point x = %d\t", mouth_center.x);
           //printf("Point y = %d\n", mouth_center.y);

            errorX = (frame.cols / 2) - mouth_center.x;
            errorY = (frame.rows / 2) - mouth_center.y;
            printf("Point x = %d\t", errorX);
            printf("Point y = %d\n", errorY);
        }
    }
    //-- Show what you got
    imshow("Capture - Face detection", frame);
}



