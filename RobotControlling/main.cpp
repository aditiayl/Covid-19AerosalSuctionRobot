#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <stdlib.h>
#include <string>
#include "SerialPort.h"

using namespace cv;
using namespace std;

char output[MAX_DATA_LENGTH];
char incomingData[MAX_DATA_LENGTH];

char port[] = "\\\\.\\COM4";
SerialPort arduino(port);

void autoconnect(void) {
	if (arduino.isConnected()) {
		cout << "Connection made" << endl << endl;
	}
	else {
		cout << "Error in port name" << endl << endl;
	}
}

void sendData(String data) {

	//cout << "data string: " << data << endl;

	char* charArray = new char[data.size() + 1];
	//cout << "charArray 1: " << data << endl;
	copy(data.begin(), data.end(), charArray);
	//cout << "charArray 2: " << data << endl;
	charArray[data.size()] = '\n';

	cout << "send: " << charArray << endl;
	arduino.writeSerialPort(charArray, 2);
	//cout << "sended: " << charArray << endl;
	arduino.readSerialPort(output, MAX_DATA_LENGTH);

	cout << "receive: " << output << endl;

	delete[] charArray;
}

int main() {
	using namespace std::this_thread; // sleep_for, sleep_until
	using namespace std::chrono; // nanoseconds, system_clock, seconds

	int min, max;
	int x, y;

	//Serial Communication
	autoconnect();


	VideoCapture cap(0);
	if (!cap.isOpened()) {
		cout << "Error opening video streamor file" << endl;
		return -1;
	}

	int flagDetect = 1;
	while (1) {
		Mat src, dst;
		cap >> src;
		if (src.empty())
			break;

		//change image to gray scale
		cvtColor(src, src, COLOR_BGR2GRAY);

		//separate the image in r g b
		vector<Mat>bgr_planes;
		split(src, bgr_planes);

		//establish the number of bins
		int histSize = 256;

		//set the range for r g b
		float range[] = { 1, 256 };
		const float* histRange = { range };

		bool uniform = true; bool accumulate = false;

		//Mat b_hist, g_hist, r_hist;
		Mat hist;

		//compute the histogram
		calcHist(&bgr_planes[0], 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);

		//draw histogram
		int hist_w = 512, hist_h = 400;
		int bin_w = cvRound((double)hist_w / histSize);

		Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(255, 255, 255));
		normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
		min = 512; max = 0;

		//draw for each channel
		int jum = 0;
		for (int i = 0; i < histSize; i++)
		{
			//line(histImage, Point(bin_w * (i - 1), hist_h - cvRound(hist.at<float>(i - 1))), Point(bin_w * (i), hist_h - cvRound(hist.at<float>(i))), Scalar(0, 0, 0), 2, 8, 0);
			//x = bin_w * (i);
			y = hist_h - cvRound(hist.at<float>(i));
			jum = jum + y;
			//printf("y=%d\n", y);
			//sleep_for(nanoseconds(50000));
			//sleep_until(system_clock::now() + seconds(1));
		}
		int rata = jum / 256;
		//printf("rata=%d\n", rata);
		String data;
		String data2;
		int k = 0;
		if (rata > 380) {
			flagDetect = 1 - flagDetect;
		}

		if(flagDetect){
			printf("Tutup (data : %d)\n", rata);
			data = "0%";
			sendData(data);
		}
		else if (!flagDetect) {
			//printf(".");
			printf("Buka (data : %d)\n", rata);
			data2 = "y%";
			sendData(data2);
		}

		//display
		//namedWindow("calcHist Demo", WINDOW_AUTOSIZE);
		//imshow("calcHist Demo", histImage);

		namedWindow("Image", WINDOW_AUTOSIZE);
		imshow("Image", src);
		char c = (char)waitKey(25);
		if (c == 27)
			break;
		//waitKey(1000);
	}
	//waitKey(0);
	cap.release();
	destroyAllWindows();


	return 0;
}




