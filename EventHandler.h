#pragma once

#include <functional>

namespace eventhandlers
{

	template<typename T>
	class EventHandler
	{
	public:
		using Func = std::function<void(T)>;
	private:
		Func _func;
	public:
		EventHandler() {}
		EventHandler(const Func &func) : _func{ func } {}
		void operator()(T multitype)
		{
			if (_func)
			{
				_func(multitype);
			}
		}
		bool operator==(const EventHandler& rhs) {
			return this == &rhs;
		}
		bool operator==(nullptr_t) {
			return this->_func == nullptr;
		}
		bool operator!=(const EventHandler& rhs) {
			return this != &rhs;
		}
		bool operator!=(nullptr_t) {
			return this->_func != nullptr;
		}
	};

}




/*

under the namespace eventhandlers
	EventHandler
this class is used to hold the function from particular class
it will only compile in vs2017 build tools

*/
