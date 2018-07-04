// MAERSK project.cpp : Defines the entry point for the console application.
//

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

// SDK files for wenglor scanner 
#include <EthernetScannerSDK.h>
#include <EthernetScannerSDKDefine.h>

// Opencv header files
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core/persistence.hpp>

// Event handler header files
#include "library/eventhandlers/Event.h" 
#include "library/eventhandlers/EventHandler.h"

// Serial communication header files
#include "SerialComms.h"

//Wenglor class
#include "Wenglor.h"

//Imageprocessing class
#include "ImageProcessing.h"

using namespace std;

//Object Declaration
Wenglor wenglor1;

//Object declaration
ImageProcessing img;

//Communication object declaration
sf::SerialComms scstart;
sf::SerialComms scstop;

bool rcv = 1;

//Recieving message

void MessageRecievedstart(std::string msg)
{
	std::cout << "Message Recieved : " << msg << std::endl;
	rcv = 0;
	wenglor1.Start();
}


void MessageRecievedstop(std::string msg)
{
	std::cout << "Message Recieved : " << msg << std::endl;
	rcv = 0;
	wenglor1.Stop();
}

int main()
{
	//Event handler for wenglor to imageprocessing
	eventhandlers::EventHandler<cv::Mat> imagefunc(std::bind(&ImageProcessing::ShowImage,img, std::placeholders::_1));
	wenglor1.processEvent += imagefunc;

	//Event handler for starting serial communication
    eventhandlers::EventHandler<std::string> msgrcv1(std::bind(MessageRecievedstart, std::placeholders::_1));
	scstart.serialEvent += msgrcv1;

	//Event handler for stopping serial communication
	eventhandlers::EventHandler<std::string> msgrcv(std::bind(MessageRecievedstop, std::placeholders::_1));
	scstop.serialEvent += msgrcv;
	
	//Intialise serial communication
	string comport = "COM6";
	scstart.Initialise(comport,9600);
	
    //Connection and image processing 
	wenglor1.Connect();
	
	scstart.Send("start1", 7);
	std::cout << "Start" << std::endl;
	while (rcv)
	{
		continue;
	}
	scstart.Close();

    cout << endl;

	rcv = 1;
	string comport1= "COM6";
	scstop.Initialise(comport1, 9600);
	scstop.Send("start", 6);
	while (rcv)
	{
		continue;
	}
	scstop.Close();
	std::cout << "move" << std::endl;
	//wenglor1.Stop();

    return 0;
}
