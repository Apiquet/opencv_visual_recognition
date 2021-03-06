#include "pch.h"
#include<iostream>

#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat src, src_gray, dilatation_result, drawing;
int min_r = 229, min_g = 239, min_b = 0;
int max_r = 255, max_g = 255, max_b = 229;
int lowThreshold = 0;
int dilation_elem = 0;
int dilation_size = 30;

int thresh = 100;
int max_thresh = 255;
RNG rng(12345);
static void colorThreshold(int, void*)
{
	inRange(src, Scalar(min_r, min_g, min_b), Scalar(max_r, max_g, max_b), src_gray);
	imshow("SRC_threshold", src_gray);
}
void thresh_callback(int, void*)
{
	Mat canny_output;
	Canny(src_gray, canny_output, thresh, thresh * 2);
	vector<vector<Point> > contours;
	findContours(canny_output, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);
	vector<vector<Point> > contours_poly(contours.size());
	vector<Rect> boundRect(contours.size());
	vector<Point2f>centers(contours.size());
	vector<float>radius(contours.size());
	for (size_t i = 0; i < contours.size(); i++)
	{
		approxPolyDP(contours[i], contours_poly[i], 3, true);
		boundRect[i] = boundingRect(contours_poly[i]);
		minEnclosingCircle(contours_poly[i], centers[i], radius[i]);
	}
	drawing = Mat::zeros(canny_output.size(), CV_8UC3);
	for (size_t i = 0; i < contours.size(); i++)
	{
		Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
		drawContours(drawing, contours_poly, (int)i, color);
		rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 20);
		circle(drawing, centers[i], (int)radius[i], color, 20);
	}
	//imshow("Contours", drawing);
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
	dilate(src_gray, dilatation_result, element);
	//imshow("Dilation", src_);
}

int main(int, char**)
{
	VideoCapture cap(0); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
		return -1;

	while (true) {


		cap >> src; // get a new frame from camera
		//cvtColor(frame, edges, COLOR_BGR2GRAY);
		//GaussianBlur(frame, edges, Size(7, 7), 2000, 2000);


		namedWindow("Color", 1);

		createTrackbar("min R:", "Color", &min_r, 255, colorThreshold);
		createTrackbar("min G:", "Color", &min_g, 255, colorThreshold);
		createTrackbar("min B:", "Color", &min_b, 255, colorThreshold);
		createTrackbar("max R:", "Color", &max_r, 255, colorThreshold);
		createTrackbar("max G:", "Color", &max_g, 255, colorThreshold);
		createTrackbar("max B:", "Color", &max_b, 255, colorThreshold);
		colorThreshold(0, 0);
		//Define your destination image
		//cv::Mat dstImage = cv::Mat::zeros(src.size(), src.type());
		thresh_callback(0, 0);
		//Now you can copy your source image to destination image with masking
		//src.copyTo(dstImage, ~src_gray);
		//cv::imshow("dstImage", dstImage);
		cv::imshow("dstImage", src + drawing);


		Dilation(0, 0);
		//Define your destination image
		cv::Mat ImageWithMask = cv::Mat::zeros(src.size(), src.type());
		//Now you can copy your source image to destination image with masking
		src.copyTo(ImageWithMask, dilatation_result);
		cv::imshow("ImageWithMask", ImageWithMask);
		
		

		waitKey(0);
		//if (waitKey(30) >= 0) break;
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}