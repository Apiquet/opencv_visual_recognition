#include "pch.h"
#include<iostream>

#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat src, src_, src_gray, drawing;
Mat dst, detected_edges, dst_threshold;
int min_r = 0, min_g = 98, min_b = 30; 
int max_r = 118, max_g = 135, max_b = 75;
int lowThreshold = 0;
const int max_lowThreshold = 255;
const int ratio_ = 3;
const int kernel_size = 3;
int erosion_elem = 0;
int erosion_size = 2;
int dilation_elem = 0;
int dilation_size = 8;
int const max_elem = 2;
int const max_kernel_size = 21;

int thresh = 100;
int max_thresh = 255;
RNG rng(12345);

static void CannyThreshold(int, void*)
{
	blur(src_gray, detected_edges, Size(3, 3));
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*ratio_, kernel_size);
	dst = Scalar::all(0);
	src.copyTo(dst, detected_edges);
}
static void colorThreshold(int, void*)
{
	inRange(src, Scalar(min_r, min_g, min_b), Scalar(max_r, max_g, max_b), src_);
	src_gray = src_;
	imshow("SRC_threshold", src_);
}
void Erosion(int, void*)
{
	int erosion_type = 0;
	if (erosion_elem == 0) { erosion_type = MORPH_RECT; }
	else if (erosion_elem == 1) { erosion_type = MORPH_CROSS; }
	else if (erosion_elem == 2) { erosion_type = MORPH_ELLIPSE; }
	Mat element = getStructuringElement(erosion_type,
		Size(2 * erosion_size + 1, 2 * erosion_size + 1),
		Point(erosion_size, erosion_size));
	erode(src_, src_, element);
	//imshow("Erosion", src_);
}
void Dilation(int, void*)
{
	int dilation_type = 0;
	if (dilation_elem == 0) { dilation_type = MORPH_RECT; }
	else if (dilation_elem == 1) { dilation_type = MORPH_CROSS; }
	else if (dilation_elem == 2) { dilation_type = MORPH_ELLIPSE; }
	Mat element = getStructuringElement(dilation_type,
		Size(2 * dilation_size + 1, 2 * dilation_size + 1),
		Point(dilation_size, dilation_size));
	dilate(src_, src_, element);
	//imshow("Dilation", src_);
}
/** @function thresh_callback */
void thresh_callback(int, void*)
{
	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	/// Detect edges using canny
	Canny(src_gray, canny_output, thresh, thresh * 2, 3);
	/// Find contours
	findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// Draw contours
	drawing = Mat::zeros(dst.size(), CV_8UC3);
	for (int i = 0; i < contours.size(); i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
	}

	/// Show in a window
	//namedWindow("Contours", WINDOW_AUTOSIZE);
	//imshow("Contours", drawing);
}
int main(int, char**)
{
	VideoCapture cap(1); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
		return -1;

	Mat edges;
	//namedWindow("edges", 1);
	for (;;)
	{
		Mat frame1, frame2, frame3;
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

		//namedWindow("edges", WINDOW_AUTOSIZE);

		/// Create Windows
		//namedWindow("Threshold", 1);

		//createTrackbar("Min Threshold:", "Threshold", &lowThreshold, max_lowThreshold, CannyThreshold);
		
		colorThreshold(0, 0);
		//cvtColor(src_, src_gray, COLOR_BGR2GRAY);
		//namedWindow("Erosion Demo", WINDOW_AUTOSIZE);
		//namedWindow("Dilation Demo", WINDOW_AUTOSIZE);
		moveWindow("Dilation Demo", src.cols, 0);
		createTrackbar("Element:\n 0: Rect \n 1: Cross \n 2: Ellipse", "Erosion Demo",
			&erosion_elem, max_elem,
			Erosion);
		createTrackbar("Kernel size:\n 2n +1", "Erosion Demo",
			&erosion_size, max_kernel_size,
			Erosion);
		createTrackbar("Element:\n 0: Rect \n 1: Cross \n 2: Ellipse", "Dilation Demo",
			&dilation_elem, max_elem,
			Dilation);
		createTrackbar("Kernel size:\n 2n +1", "Dilation Demo",
			&dilation_size, max_kernel_size,
			Dilation);
		Erosion(0, 0);
		Dilation(0, 0);
		//CannyThreshold(0, 0);
		createTrackbar(" Canny thresh:", "Source", &thresh, max_thresh, thresh_callback);
		thresh_callback(0, 0);
		imshow("RESULT", src + drawing);
		if (waitKey(30) >= 0) break;
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}