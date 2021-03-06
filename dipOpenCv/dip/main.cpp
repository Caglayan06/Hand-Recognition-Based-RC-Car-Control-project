#include "stdafx.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "SerialClass.h"
#include <thread>

bool threadFinished = false;
Serial* port;


using namespace cv;
using namespace std;
void initBT();
void sendDataBT(int, int);

int main(int argc, const char** argv) {
	//initBT();
		
	char arr[40];
	VideoCapture cam(0);

	if (!cam.isOpened()) {
		return -1;
	}

	Mat img;
	Mat img_threshold;
	Mat img_gray;
	Mat img_roi;
	namedWindow("Orijinal", CV_WINDOW_AUTOSIZE);
	namedWindow("Threshold", CV_WINDOW_AUTOSIZE);

	while (true) {
		bool ret = cam.read(img);
		if (!ret) {
			return -1;
		}
		Rect roi(270, 50, 340, 400);
		img_roi = img(roi);
		cvtColor(img_roi, img_gray, CV_RGB2GRAY);

		GaussianBlur(img_gray, img_gray, Size(19, 19), 0.0, 0);
		threshold(img_gray, img_threshold, 0, 255, THRESH_BINARY_INV + THRESH_OTSU);

		vector<vector<Point> >contours;
		vector<Vec4i>hierarchy;
		findContours(img_threshold, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point());
		if (contours.size()>0) {
			size_t indexOfBiggestContour = -1;
			size_t sizeOfBiggestContour = 0;

			for (size_t i = 0; i < contours.size(); i++) {
				if (contours[i].size() > sizeOfBiggestContour) {
					sizeOfBiggestContour = contours[i].size();
					indexOfBiggestContour = i;
				}
			}
			vector<vector<int> >hull(contours.size());
			vector<vector<Point> >hullPoint(contours.size());
			vector<vector<Vec4i> >defects(contours.size());
			vector<vector<Point> >defectPoint(contours.size());
			vector<vector<Point> >contours_poly(contours.size());
			Point2f rect_point[4];
			vector<RotatedRect>minRect(contours.size());
			vector<Rect> boundRect(contours.size());
			for (size_t i = 0; i<contours.size(); i++) {
				if (contourArea(contours[i])>5000) {
					convexHull(contours[i], hull[i], true);
					convexityDefects(contours[i], hull[i], defects[i]);
					if (indexOfBiggestContour == i) {
						minRect[i] = minAreaRect(contours[i]);
						for (size_t k = 0; k<hull[i].size(); k++) {
							int ind = hull[i][k];
							hullPoint[i].push_back(contours[i][ind]);
						}

						for (size_t k = 0; k<defects[i].size(); k++) {
							if (defects[i][k][3]>13 * 256) {
								int p_end = defects[i][k][1];
								int p_far = defects[i][k][2];
								defectPoint[i].push_back(contours[i][p_far]);
								circle(img_roi, contours[i][1], 20, Scalar(0, 0, 255), 3);
								sprintf_s(arr, "%d,%d", contours[i][1].x, contours[i][1].y);
								putText(img, arr, Point(70, 70), CV_FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 0, 0), 1, 8, false);
									
								
								/*if (port->IsConnected()) {
									//thread sendDataThread(sendDataBT, contours[i][1].x, contours[i][1].y);
									//sendDataThread.join();
									//Sleep(10);
									sendDataBT(contours[i][1].x, contours[i][1].y);
									cout << contours[i][1].x << contours[i][1].y << endl;
									
								}
								else {
									port = new Serial("COM5");
									if (port->IsConnected()) cout << "Connected!" << endl;
								}
								
								*/

								
							}
						}


						drawContours(img_threshold, contours, i, Scalar(255, 255, 0), 2, 8, vector<Vec4i>(), 0, Point());
						drawContours(img_threshold, hullPoint, i, Scalar(255, 255, 0), 1, 8, vector<Vec4i>(), 0, Point());
						//drawContours(img_roi, hullPoint, i, Scalar(0, 0, 255), 2, 8, vector<Vec4i>(), 0, Point());
						boundRect[i] = boundingRect(contours_poly[i]);
						//rectangle(img_roi, boundRect[i].tl(), boundRect[i].br(), Scalar(255, 0, 0), 2, 8, 0);
						minRect[i].points(rect_point);
						//for (size_t k = 0; k<4; k++) {
						//	line(img_roi, rect_point[k], rect_point[(k + 1) % 4], Scalar(0, 255, 0), 2, 8);
						//}

					}
				}
				line(img, Point(340, 100), Point(340, 370), Scalar(255, 0, 0), 1, 8, 0);
				line(img, Point(610, 100), Point(610, 370), Scalar(255, 0, 0), 1, 8, 0);
				line(img, Point(340, 100), Point(610, 100), Scalar(255, 0, 0), 1, 8, 0);
				line(img, Point(340, 370), Point(610, 370), Scalar(255, 0, 0), 1, 8, 0);

				line(img, Point(340, 200), Point(610, 200), Scalar(255, 0, 0), 1, 8, 0);
				line(img, Point(340, 270), Point(610, 270), Scalar(255, 0, 0), 1, 8, 0);
				line(img, Point(420, 100), Point(420, 370), Scalar(255, 0, 0), 1, 8, 0);
				line(img, Point(530, 100), Point(530, 370), Scalar(255, 0, 0), 1, 8, 0);


				line(img, Point(270, 50), Point(610, 50), Scalar(0, 255, 0), 2, 8, 0);
				line(img, Point(270, 50), Point(270, 450), Scalar(0, 255, 0), 2, 8, 0);
				line(img, Point(270, 450), Point(610, 450), Scalar(0, 255, 0), 2, 8, 0);
				line(img, Point(610, 50), Point(610, 450), Scalar(0, 255, 0), 2, 8, 0);
			}
			imshow("Orijinal", img);
			imshow("Threshold", img_threshold);
			if (waitKey(30) == 27) {
				return -1;
			}
		}
	}
	
system("pause");
	return 0;
}

void initBT() {
	//start bluetooth
	port = new Serial("COM5");
	if (port->IsConnected()) cout << "Connected!" << endl;
}

void sendDataBT(int x, int y) {
	char command[7] = "";


	//stop
	if ((x >= 150 && x < 260) && (y >= 150 && y < 220)) {
		sprintf_s(command, "20,40 "); 
		port->WriteData(command, 7);
		Sleep(20);
	}  	

	//forward
	else if ((x >= 150 && x < 260) && (y > 50 && y <= 150)) {
		int val = 10;
		int newy;

		newy =(int)((y-50) / 10.f);
		newy = val - newy;


		sprintf_s(command, "%d,40 ", newy);

		port->WriteData(command, 7);
		Sleep(20);
	}

	//back
	else if ((x >= 150 && x < 260) && (y >= 220 && y < 320)) {

		int val = 30;
		int newy;

		newy = (int)((320-y) / 10.f);

		newy = val - newy;
		sprintf_s(command, "%d,40 ", newy);

		port->WriteData(command, 7);
		Sleep(20);
	}

	//left
	else if ((x >= 250 && x < 340) && (y >= 150 && y < 220)) {
		sprintf_s(command, "20,50 ");
		port->WriteData(command, 7);
		Sleep(20);
	}

	//right
	else if ((x >= 60 &&  x < 150) && (y > 150 && y <= 220)) {
		sprintf_s(command, "20,60 ");
		port->WriteData(command, 7);
		Sleep(20);
	}

	//forward-left
	else if ((x > 260 && x <= 340) && (y >= 50 && y < 150)) {
		int val = 10;
		int newy;

		newy = (int)((y-50) / 10.f);
		newy = val - newy;

		sprintf_s(command, "%d,50 ", newy);

		port->WriteData(command, 7);
		Sleep(20);
	}

	//forward-right
	else if ((x > 60 && x <= 150) && (y > 50 && y <= 150)) {
		int val = 10;
		int newy;

		newy = (int)((y - 50) / 10.f);
		newy = val - newy;

		sprintf_s(command, "%d,60 ", newy);

		port->WriteData(command, 7);
		Sleep(20);
	}

	//back-left
	else if ((x >= 250 && x < 340) && (y > 220 && y <= 320)) {

		int val = 30;
		int newy;

		newy = (int)((320 - y) / 10.f);

		newy = val - newy;
		sprintf_s(command, "%d,50 ", newy);

		port->WriteData(command, 7);
		Sleep(20);
	}

	//back-right
	else if ((x >= 70 && x < 150) && (y >= 220 && y < 320)) {
		int val = 30;
		int newy;

		newy = (int)((320 - y) / 10.f);

		newy = val - newy;
		sprintf_s(command, "%d,60 ", newy);

		port->WriteData(command, 7);
		Sleep(20);
	} 
	else {
			sprintf_s(command, "20,40 ");
			port->WriteData(command, 7);
			Sleep(20);
		}

}
