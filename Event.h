#pragma once
#include <vector>
#include <mutex>
#include <algorithm>
#include <memory>

#include "EventHandler.h"

namespace eventhandlers
{
	template<typename T>
	class Event
	{
	private:
		std::vector<EventHandler<T>*> _allHandler;
		std::mutex _allHandler_mutex;
		bool _removeNulls = false;
		void RemoveNulls() {
			if (_removeNulls)
			{
				_allHandler.erase(std::remove_if(_allHandler.begin(), _allHandler.end(), [](EventHandler<T>* ptr) {return *ptr == NULL; }), _allHandler.end());
				_removeNulls = false;
			}
		}
	public:
		Event() {}
		~Event() {}
		void AddHandler(EventHandler<T>& handler)
		{
			std::lock_guard<std::mutex> lock(_allHandler_mutex);
			if (std::find(_allHandler.begin(), _allHandler.end(), &handler) == _allHandler.end())
			{
				_allHandler.push_back(&handler);
			}
		}
		void RemoveHandler(EventHandler<T>& handler)
		{
			std::lock_guard<std::mutex> lock(_allHandler_mutex);
			_allHandler.erase(std::remove(_allHandler.begin(), _allHandler.end(), &handler), _allHandler.end());
		}
		void RemoveAllHandler() 
		{
			std::lock_guard<std::mutex> lock(_allHandler_mutex);
			_allHandler.clear();
		}
		void FireEvent(T multitype)
		{
			std::lock_guard<std::mutex> lock(_allHandler_mutex);
			for (EventHandler<T>* fn : _allHandler)
			{
				EventHandler<T> tmpFnc = *fn;
				if (tmpFnc != NULL)
				{
					try
					{
						tmpFnc(multitype);
					}
					catch (const std::exception&)
					{

					}
				}
				else
				{
					_removeNulls = true;
				}
			}
			RemoveNulls();
		}

		Event &operator+=(EventHandler<T>& handler) {
			this->AddHandler(handler);
			return *this;
		}

		Event &operator-=(EventHandler<T>& handler) {
			this->RemoveHandler(handler);
			return *this;
		}

		void operator()(T multitype) {
			this->FireEvent(multitype);
		}

	};

}




/*
under the namespace eventhandlers
	Event
is used to hold the EventHandler pointers in vector and call that function in serial manner when ever the FireEvent called
once the pointer is no longer valide it is automatically removed from the vector list
*/