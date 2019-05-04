#include "pch.h"
#include<iostream>

#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat src, src_, src_gray, drawing;
Mat dst, detected_edges, dst_threshold;
int min_r = 0, min_g = 0, min_b = 0; 
int max_r = 121, max_g = 157, max_b = 55;
int lowThreshold = 0;
const int max_lowThreshold = 255;
const int ratio_ = 3;
const int kernel_size = 3;
int erosion_elem = 0;
int erosion_size = 2;
int dilation_elem = 0;
int dilation_size = 18;
int const max_elem = 2;
int const max_kernel_size = 21;

int thresh = 100;
int max_thresh = 255;
RNG rng(12345);
void detect_text(Mat rgb) {

	//Mat rgb;
	// downsample and use it for processing
	//pyrDown(large, rgb);
	//pyrDown(rgb, rgb);
	Mat small;
	cvtColor(rgb, small, COLOR_BGR2GRAY);
	//small = rgb;
	// morphological gradient
	Mat grad;
	Mat morphKernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	morphologyEx(small, grad, MORPH_GRADIENT, morphKernel);
	// binarize
	Mat bw;
	threshold(grad, bw, 0.0, 255.0, THRESH_BINARY | THRESH_OTSU);
	// connect horizontally oriented regions
	Mat connected;
	morphKernel = getStructuringElement(MORPH_RECT, Size(9, 1));
	morphologyEx(bw, connected, MORPH_CLOSE, morphKernel);
	// find contours
	Mat mask = Mat::zeros(bw.size(), CV_8UC1);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(connected, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE, Point(0, 0));
	// filter contours
	for (int idx = 0; idx >= 0; idx = hierarchy[idx][0]) {
		Rect rect = boundingRect(contours[idx]);
		Mat maskROI(mask, rect);
		maskROI = Scalar(0, 0, 0);
		// fill the contour
		drawContours(mask, contours, idx, Scalar(255, 255, 255), FILLED);

		RotatedRect rrect = minAreaRect(contours[idx]);
		double r = (double)countNonZero(maskROI) / (rrect.size.width * rrect.size.height);

		Scalar color;
		int thickness = 1;
		// assume at least 25% of the area is filled if it contains text
		if (r > 0.25 &&
			(rrect.size.height > 8 && rrect.size.width > 8 && rrect.size.height < 50 && rrect.size.width < 50) // constraints on region size
			// these two conditions alone are not very robust. better to use something 
			//like the number of significant peaks in a horizontal projection as a third condition
			) {
			thickness = 2;
			color = Scalar(0, 255, 0);
		}
		else
		{
			thickness = 1;
			color = Scalar(0, 0, 255);
			continue;
		}

		Point2f pts[4];
		rrect.points(pts);
		for (int i = 0; i < 4; i++)
		{
			line(rgb, Point((int)pts[i].x, (int)pts[i].y), Point((int)pts[(i + 1) % 4].x, (int)pts[(i + 1) % 4].y), color, thickness);
		}
	}

	cv::imshow("txt", rgb);
}
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
	//imshow("SRC_threshold", src_);
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
	while (true) {
		
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
		namedWindow("Erosion Demo", WINDOW_AUTOSIZE);
		namedWindow("Dilation Demo", WINDOW_AUTOSIZE);
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
		//createTrackbar(" Canny thresh:", "Source", &thresh, max_thresh, thresh_callback);
		//thresh_callback(0, 0);
			

		//Define your destination image
		cv::Mat dstImage = cv::Mat::zeros(src.size(), src.type());
		//Now you can copy your source image to destination image with masking
		src.copyTo(dstImage, src_);
		cv::imshow("dstImage", src);

		//detect_text(dstImage);

		if (waitKey(30) >= 0) break;
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}