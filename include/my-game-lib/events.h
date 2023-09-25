#ifndef __MY_GAME_LIB_EVENTS_HEADER_H__
#define __MY_GAME_LIB_EVENTS_HEADER_H__

#include <string_view>
#include <string_view>

#include <cstdint>

#include <my-lib/macros.h>
#include <my-lib/std.h>
#include <my-lib/trigger.h>
#include <my-lib/any.h>
#include <my-lib/memory.h>

namespace MyGlib
{

// ---------------------------------------------------

class EventManager
{
protected:
	Mylib::Memory::Manager& memory_manager;
	
public:
	EventManager (Mylib::Memory::Manager& memory_manager_)
		: memory_manager(memory_manager_)
	{
	}

	virtual void check_events () = 0;
};

// ---------------------------------------------------

} // end namespace MyGlib

#endif