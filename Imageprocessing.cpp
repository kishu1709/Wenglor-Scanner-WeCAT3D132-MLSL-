// Header files
#include "stdafx.h"
#include <iostream>
#include <string>
#include <exception>
#include <thread>
#include <chrono>
#include <queue>
#include <numeric>
#include <stdlib.h>
#include <csignal>

// Opencv header files
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core/persistence.hpp>

// Imageprocessing header file
 #include "ImageProcessing.h"

using namespace std;
using namespace cv;

ImageProcessing::ImageProcessing()
{
}

ImageProcessing::~ImageProcessing()
{
}

//======================================================================================================================//
///Displaying the image 
//======================================================================================================================//
void ImageProcessing :: ShowImage(cv::Mat image)
{
	imshow("display1", image);
	waitKey(0);

	//Normalise image
	Mat image1;
	NormaliseFrame(image, image1);

	//Display results
	imshow("display2", image1);
	waitKey(0);

	Process(image1);
	ErrorTime(x);
	destroyAllWindows();
}

//======================================================================================================================//
///Normalizing image for clear view
//======================================================================================================================//
void ImageProcessing:: NormaliseFrame(const cv::Mat& frame, cv::Mat& normalised)
{
	// get the minimum and maximum value from the image and normalise
	double min, max;
	cv::Point minPoint, maxPoint;
	cv::Mat normalised1;

	normalised = cv::Mat(frame.rows, frame.cols, CV_64FC1, 0.0);
	cv::minMaxLoc(frame, &min, &max, &minPoint, &maxPoint);

	// taking the minimum value as hardcoded to see the texture very carefully
	//min = 0 ;
	if (min != max && max > 0.75)
	{
		normalised1 = frame.clone();

		normalised1 -= min;
		normalised1.convertTo(normalised, CV_8U, 255.0 / (max - min));
	}
}

//======================================================================================================================//
///Finding the errors
//======================================================================================================================//

vector<double> ImageProcessing:: Process(Mat image1)
{
	//Scalar avg, std;
	//meanStdDev(image1, avg, std
	int min,max;
	cout << "Enter the minimum value:" << endl;
	cin >> min;
	cout << "Enter the maximum value:" << endl;
	cin >> max;
	double val = 0.0;
	int nrows = image1.rows;
	int ncols = image1.cols;

	int i, j;
	double l;
	//double* p;
	for (i = 0; i < nrows; ++i)
	{
		for (j = 0; j < ncols; ++j)
		{
			val = image1.at<double>(i, j);
			if (val<min || val>max)
			{
				l = (double(i) * 5 / nrows);
				x.push_back(l);
				break;
			}
		}
	}
	return x;
}

//======================================================================================================================//
///printing the time of error
//======================================================================================================================//

void ImageProcessing::ErrorTime(vector<double> x)
{
	int temp = 0;
	for (int k = 0; k < x.size() - 1; k++)
	{
		if ((x[k + 1] - x[k]) < 0.002)
		{
			v.push_back(x[k]);
		}
		else
		{
			v.push_back(x[k]);
			cout << v[0] << " to " << v[v.size() - 1] << endl;
			v.clear();
		}
	}
}