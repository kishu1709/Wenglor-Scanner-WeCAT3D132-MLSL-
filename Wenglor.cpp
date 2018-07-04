// Header files
#include "stdafx.h"
#include <iostream>
#include <string>
#include <exception>
#include <thread>
#include <chrono>
#include <numeric>
#include <stdlib.h>
#include <csignal>
#include <vector>

// Opencv header files
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>

// SDK files for wenglor scanner 
#include <EthernetScannerSDK.h>
#include <EthernetScannerSDKDefine.h>

// Event handler header files
#include "library/eventhandlers/Event.h"
#include "library/eventhandlers/EventHandler.h"

//Wenglor header file
#include "Wenglor.h"

using namespace std;
using namespace cv;

//======================================================================================================================//
///Function Definition
//======================================================================================================================//

//======================================================================================================================//
/// Control scanner functions 
//======================================================================================================================//

bool Wenglor::GetScannerActive() { return activeScanner; }

void Wenglor::SetScannerActive() { activeScanner = true; }

void Wenglor::SetScannerInactive() { activeScanner = false; }

Wenglor::Wenglor()
{}

Wenglor::~Wenglor()
{}

//======================================================================================================================//
///Establish Connection definition
//======================================================================================================================//

void Wenglor::Connect()
{
	char IP[32] = "192.168.001.250";
	char Port[32] = "32001";
	/*cin >> IP;
	cin >> Port;*/
	scanner = EthernetScanner_Connect(IP, Port, 1000);
	ConsolidateScannerConnection();
}

void Wenglor::ConsolidateScannerConnection()
{
	auto timenow = chrono::system_clock::now().time_since_epoch();
	auto t100ms = chrono::milliseconds(3000);
	auto timeout = timenow + t100ms;
	int status = 0;
	while (timenow <= timeout)
	{
		EthernetScanner_GetConnectStatus(scanner, &status);
		if (ETHERNETSCANNER_TCPSCANNERCONNECTED == status)
			break;

		timenow = chrono::system_clock::now().time_since_epoch();
	}

	if (timenow > timeout)
		cout << "Failed to connect to scanner "<<endl;

	// Transfer linearisation data
	timenow = chrono::system_clock::now().time_since_epoch();
	t100ms = chrono::milliseconds(10000);
	timeout = timenow + t100ms;
	status = -1;
	const int maxInfoSize = 128 * 1024;
	char scannerInfo[maxInfoSize] = { 0 };
	while (timenow <= timeout)
	{
		// Transfer of the linearisation table can be checked by calling the
		// GetInfo function

		status = EthernetScanner_GetInfo(scanner, scannerInfo, maxInfoSize, (char *)"text");
		if (status > 0)
		{
			break;
		}
		timenow = chrono::system_clock::now().time_since_epoch();
	}

	if (timenow > timeout)
		cout << "Failed to get info for scanner "<<endl;
}

//======================================================================================================================//
///Start Capturing images
//======================================================================================================================//

void Wenglor::Start()
{
	runThread.reset(new thread(std::bind(&Wenglor::Run, std::ref(*this))));
}

//======================================================================================================================//
///Run thread
//======================================================================================================================//

void Wenglor::Run()
{
	EthernetScanner_WriteData(scanner, (char *)"SetAcquisitionStart", sizeof("SetAcquisitionStart"));
	SetScannerActive();

	image = cv::Mat();
	while (GetScannerActive())
		// Get the scanner's output
	{
		// Check connection
		int status = -1;
		EthernetScanner_GetConnectStatus(scanner, &status);
		if (ETHERNETSCANNER_TCPSCANNERCONNECTED != status)
			break;
		double bx[8192] = { 0 };
		double bz[8192] = { 0 };
		int bi[8192] = { 0 };
		int peakWidth[8192] = { 0 };
		const int bufferSize = ETHERNETSCANNER_SCANXMAX * ETHERNETSCANNER_PEAKSPERCMOSSCANLINEMAX;
		unsigned int encoder = 0;
		unsigned char userio = 0;
		int picCount = 0;

		int numPoints = EthernetScanner_GetXZIExtended(scanner, bx, bz, bi, peakWidth, bufferSize, &encoder, &userio, 0, NULL, 0, &picCount);
		if (numPoints <= 0)
		{
			//cout << "number of points 0";
			this_thread::sleep_for(chrono::milliseconds(1));
			continue;
		}
		vector<double> vz;
		vz.insert(vz.end(), std::begin(bz), std::begin(bz) + 1280);

		Mat line(vz, false);
		Mat lineTranspose = line.t();

		if (image.empty())
		{
			image = lineTranspose;
			continue;
		}

		image.push_back(lineTranspose);
	}
}

//==========================================================================================
/// Stop Capturing images
//==============================================================================================

void Wenglor::Stop()
{
	Mat dst = image.clone();
	processEvent(dst.clone());
	SetScannerInactive();
	EthernetScanner_WriteData(scanner, (char *)"SetAcquisitionStop", sizeof("SetAcquisitionStop"));
	if (runThread->joinable())
		runThread->join();

}

//======================================================================================================================//
///Removing Connection definition
//======================================================================================================================//

void Wenglor::Disconnect()
{
	EthernetScanner_Disconnect(scanner);
}