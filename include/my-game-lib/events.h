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

enum class Key
{
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	Space,
	Return,
	Unknown
};

// ---------------------------------------------------

struct EventKeyDown {
	Key key;
};

// ---------------------------------------------------

struct EventQuit {
};

// ---------------------------------------------------

class EventManager
{
protected:
	Mylib::Memory::Manager& memory_manager;
	Mylib::Trigger::EventHandler<EventKeyDown> event_key_down;
	Mylib::Trigger::EventHandler<EventQuit> event_quit;
	
public:
	EventManager (Mylib::Memory::Manager& memory_manager_)
		: memory_manager(memory_manager_)
	{
	}

	virtual void process_events () = 0;

	Mylib::Trigger::EventHandler<EventKeyDown>& key_down ()
	{
		return this->event_key_down;
	}

	Mylib::Trigger::EventHandler<EventQuit>& quit ()
	{
		return this->event_quit;
	}
};

// ---------------------------------------------------

} // end namespace MyGlib

#endif