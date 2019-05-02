#include "pch.h"
#include<iostream>

#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat src, src_, src_gray;
Mat dst, detected_edges, dst_threshold;
int min_r = 0, min_g = 65, min_b = 30; 
int max_r = 118, max_g = 135, max_b = 83;
int lowThreshold = 0;
const int max_lowThreshold = 255;
const int ratio_ = 3;
const int kernel_size = 3;

static void CannyThreshold(int, void*)
{
	blur(src_gray, detected_edges, Size(3, 3));
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*ratio_, kernel_size);
	dst = Scalar::all(0);
	src.copyTo(dst, detected_edges);
	imshow("edges", dst);
}
static void colorThreshold(int, void*)
{
	inRange(src, Scalar(min_r, min_g, min_b), Scalar(max_r, max_g, max_b), src_);
	src_gray = src_;
	imshow("SRC", src);
	imshow("SRC_threshold", src_);
}

int main(int, char**)
{
	VideoCapture cap(1); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
		return -1;

	Mat edges;
	namedWindow("edges", 1);
	for (;;)
	{
		Mat frame;
		cap >> src; // get a new frame from camera
		//cvtColor(frame, edges, COLOR_BGR2GRAY);
		//GaussianBlur(frame, edges, Size(7, 7), 2000, 2000);

		//Canny(edges, edges, 0, 20, 3);
		dst.create(src.size(), src.type());
		/// Create Windows
		namedWindow("Color", 1);

		createTrackbar("min R:", "Color", &min_r, 255, colorThreshold);
		createTrackbar("min G:", "Color", &min_g, 255, colorThreshold);
		createTrackbar("min B:", "Color", &min_b, 255, colorThreshold);
		createTrackbar("max R:", "Color", &max_r, 255, colorThreshold);
		createTrackbar("max G:", "Color", &max_g, 255, colorThreshold);
		createTrackbar("max B:", "Color", &max_b, 255, colorThreshold);

		namedWindow("edges", WINDOW_AUTOSIZE);

		/// Create Windows
		namedWindow("Threshold", 1);

		createTrackbar("Min Threshold:", "Threshold", &lowThreshold, max_lowThreshold, CannyThreshold);
		
		colorThreshold(0, 0);
		//cvtColor(src_, src_gray, COLOR_BGR2GRAY);
		CannyThreshold(0, 0);
		
		if (waitKey(30) >= 0) break;
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}