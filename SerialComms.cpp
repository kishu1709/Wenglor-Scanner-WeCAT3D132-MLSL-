
//=============================================================================
/// \file		SerialComms.cpp
///
/// Generic class for serial communication in Windows
///
/// \date		24 October 2016
/// \author		S. Sundaram
/// \copyright	Spookfish
//=============================================================================

#include "stdafx.h"
#include "SerialComms.h"
#include "Exceptions.h"
#include <iostream>
#include <winerror.h>
#include <thread>

// Static variables
bool sf::SerialComms::connected = false;

//************************************
// Method:    SerialComms
// FullName:  sf::SerialComms::SerialComms
// Access:    public 
// Returns:   
// Qualifier:
// Parameter: void
//************************************
sf::SerialComms::SerialComms(void)
{
	messageSignal.reset(new boost::signals2::signal<void(std::string)>());
}

//************************************
// Method:    ~SerialComms
// FullName:  sf::SerialComms::~SerialComms
// Access:    virtual public 
// Returns:   
// Qualifier:
// Parameter: void
//************************************
sf::SerialComms::~SerialComms(void)
{
	Close();
}

//************************************
// Method:    Initialise
// FullName:  sf::SerialComms::Initialise
// Access:    virtual public 
// Returns:   void
// Qualifier:
// Parameter: const std::string comPort
// Parameter: const int baudRate
// Parameter: const int parity
// Parameter: const int byteSize
// Parameter: const int stopBits
//************************************
void sf::SerialComms::Initialise(const std::string comPort,
								 const int baudRate /*= 151200*/,
								 const int parity /*= 0*/,
								 const int byteSize /*= 8*/,
								 const int stopBits /*= 0*/)
{
	// Open port
	const std::string portName = "\\\\.\\" + comPort;
	port = CreateFile(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (INVALID_HANDLE_VALUE == port)
	{
		int i = WSAGetLastError () ;
		std :: cout << " i = " << i << std :: endl ;
		return;
	}

	// Set properties
	DCB serialParams = { 0 };
	serialParams.DCBlength = sizeof(serialParams);

	GetCommState(port, &serialParams);
	serialParams.BaudRate = baudRate;
	serialParams.Parity = parity;
	serialParams.ByteSize = byteSize;
	serialParams.StopBits = stopBits;
	SetCommState(port, &serialParams);

	// Set timeouts
	COMMTIMEOUTS timeout = { 0 };
	timeout.ReadIntervalTimeout = 20;
	timeout.ReadTotalTimeoutConstant = 20;
	timeout.ReadTotalTimeoutMultiplier = 2;
	timeout.WriteTotalTimeoutConstant = 1;
	timeout.WriteTotalTimeoutMultiplier = 1;
	SetCommTimeouts(port, &timeout);

	connected = true;

	//messageSignal.reset ( new boost::signals2::signal< void(std::string) > ) ;

	//Start listening for incoming messages
	std::thread listener(std::bind(&sf::SerialComms::Listen, std::ref(*this)));
	listener.detach ( ) ;
}

//************************************
// Method:    Send
// FullName:  sf::SerialComms::Send
// Access:    virtual public 
// Returns:   void
// Qualifier:
// Parameter: const char * msg
// Parameter: const int length
//************************************
void sf::SerialComms::Send(const char *msg, const int length)
{
	PurgeComm(port, PURGE_RXCLEAR);

	// Check port
	if (NULL == port || INVALID_HANDLE_VALUE == port)
		return;
		// sf::ThrowEx("Attempted to send message to invalid serial port");

	DWORD numBytes = 0;
	WriteFile(port, msg, length, &numBytes, NULL);

	if (int(numBytes) < length)
	{
		MessageBox(NULL, "Error is in sending to the serial port", "INFO", MB_OK);
		sf::ThrowEx("Failed to write message into serial port");
	}
}

//************************************
// Method:    Listen
// FullName:  sf::SerialComms::Listen
// Access:    virtual public 
// Returns:   void
// Qualifier:
//************************************
void sf::SerialComms::Listen()
{
	try
	{
		PurgeComm(port, PURGE_RXCLEAR);
		while (connected)
		{
			char msg[256] = {0};
			DWORD numBytes = 0;

			if (INVALID_HANDLE_VALUE == port)
			{

				int i = WSAGetLastError () ;
				std :: cout << " i = " << i << std :: endl ;
				return;
			}

			// Blocking call to read any incoming messages on the port
			ReadFile(port, &msg, 255, &numBytes, NULL);

			// If ReadFile returns with 0 bytes read, then the port is closed
			if (0 == numBytes)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				continue;
			}

			else
			{
				serialEvent(std::string (msg));
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				break ;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}

	catch(std::exception e)
	{
		MessageBox(NULL, std::string(std::string("Listening failed: ")+ e.what()).c_str(), "SERIAL COMM ERROR", MB_ICONERROR);
	}
}

//************************************
// Method:    Close
// FullName:  sf::SerialComms::Close
// Access:    virtual public 
// Returns:   void
// Qualifier:
//************************************
void sf::SerialComms::Close()
{
	if (NULL != port && INVALID_HANDLE_VALUE != port)
		CloseHandle(port);

	connected = false;
	port = NULL;
}

//=============================================================================
// \brief		Set the timeout for receiving messages to the specified
//				interval in milliseconds
//
// \param[in]	milliseconds		Timeout in milliseconds
//									If 0, default values are used
//=============================================================================
void sf::SerialComms::SetReadTimeout(const int milliseconds /* = 0 */)
{
	// Set timeouts
	COMMTIMEOUTS timeout = { 0 };
	timeout.ReadIntervalTimeout = milliseconds == 0 ? 20 : MAXDWORD;
	timeout.ReadTotalTimeoutConstant = milliseconds == 0 ? 20 : milliseconds;
	timeout.ReadTotalTimeoutMultiplier = milliseconds == 0 ? 2 : MAXDWORD;

	timeout.WriteTotalTimeoutConstant = 1;
	timeout.WriteTotalTimeoutMultiplier = 1;

	SetCommTimeouts(port, &timeout);
}

//=============================================================================
// \brief		Wait for a single acknowledgement message	
//=============================================================================
bool sf::SerialComms::GetAcknowledgement(const std::string &ackMsg)
{
	SetReadTimeout(1000);

	char msg[256] = { 0 };
	DWORD numBytes = 0;

	// Blocking call to read any incoming messages on the port
	ReadFile(port, &msg, 255, &numBytes, NULL);

	// Reset read timeout
	SetReadTimeout();

	// If nothing the read times out, throw an error
	if (0 >= numBytes)
		return false;

	// Else, check if the acknowledgement message matches
	if (std::string::npos != std::string(msg).find(ackMsg))
		return true;
}
