#pragma once

//=============================================================================
/// \file		SerialComms.h
///
/// Generic class for serial communication in Windows
///
/// \date		24 October 2016
/// \author		S. Sundaram
/// \copyright	Spookfish
//=============================================================================

#include <string>
//#include <boost/signals2/signal.hpp>
//#include <boost/shared_ptr.hpp>
#include <Windows.h>
#include "library/eventhandlers/EventHandler.h"
#include "library/eventhandlers/Event.h"

namespace sf
{
	class SerialComms
	{
	public:

		/// Default constructor
		SerialComms(void);

		/// Default destructor
		virtual ~SerialComms(void);

		/// Initialise communication over specifed port
		virtual void Initialise(const std::string comPort,
			const int baudRate = 115200,
			const int parity = 0,
			const int byteSize = 8,
			const int stopBits = 0);

		/// Send a message via the serial port
		virtual void Send(const char *msg, const int length);

		/// Listen for messages over the serial port
		virtual void Listen();

		/// Close communication over the current port
		virtual void Close();

		/// Connection status
		bool IsConnected() { return connected; }

		/// Set timeout on read
		virtual void SetReadTimeout(const int milliseconds = 0);

		/// Get a single acknowledgement message
		virtual bool GetAcknowledgement(const std::string &ackMsg);

		eventhandlers::Event<std ::string> serialEvent;

		/// Signal to communicate received message
		//boost::shared_ptr<boost::signals2::signal<void(std::string)> > messageSignal;

	private:

		/// Port handle
		HANDLE port;

		/// Connection status
		static bool connected;
	};
}
