#pragma once
// Header files
#include "stdafx.h"
#include <iostream>
#include <string>
#include <vector>

// Opencv header files
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>

using namespace std;

//======================================================================================================================//
///ImageProcessing Class Declaration
//======================================================================================================================//

class ImageProcessing
{
public:
	cv::Mat image;
	vector<double> x;
	vector<double> v;

	//Constructor
	ImageProcessing();

	//Destructor
	~ImageProcessing();

	//Showing and saving the image
	void ShowImage(cv::Mat);

	//Normalizing image for clear view
	void NormaliseFrame(const cv::Mat&, cv::Mat&);

	//Finding the errors
	vector<double> Process(cv::Mat);

	//printing the time of error
	void ErrorTime(vector<double>);
};