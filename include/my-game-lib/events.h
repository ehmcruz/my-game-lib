#ifndef __MY_GAME_LIB_EVENTS_HEADER_H__
#define __MY_GAME_LIB_EVENTS_HEADER_H__

#include <string_view>
#include <string_view>

#include <cstdint>

#include <SDL.h>

#include <my-lib/macros.h>
#include <my-lib/std.h>
#include <my-lib/trigger.h>
#include <my-lib/any.h>
#include <my-lib/memory.h>

namespace MyGlib
{
namespace Event
{

// ---------------------------------------------------

struct EmptyEvent {
};

// ---------------------------------------------------

struct KeyDown {
	SDL_Keycode key_code;
	SDL_Scancode scan_code;
	Uint16 modifiers;
};

// ---------------------------------------------------

struct TouchScreenMoveData {
	#define _MYGLIB_ENUM_CLASS_DIRECTION_VALUES_ \
		_MYGLIB_ENUM_CLASS_DIRECTION_VALUE_(Left) \
		_MYGLIB_ENUM_CLASS_DIRECTION_VALUE_(Right) \
		_MYGLIB_ENUM_CLASS_DIRECTION_VALUE_(Up) \
		_MYGLIB_ENUM_CLASS_DIRECTION_VALUE_(Down) \
		_MYGLIB_ENUM_CLASS_DIRECTION_VALUE_(Stopped)   // must be the last one

	enum class Direction : uint8_t {
		#define _MYGLIB_ENUM_CLASS_DIRECTION_VALUE_(V) V,
		_MYGLIB_ENUM_CLASS_DIRECTION_VALUES_
		#undef _MYGLIB_ENUM_CLASS_DIRECTION_VALUE_
	};

	Direction direction;
};

const char* enum_class_to_str (const TouchScreenMoveData::Direction value);

inline std::ostream& operator << (std::ostream& out, const TouchScreenMoveData::Direction value)
{
	out << enum_class_to_str(value);
	return out;
}

// ---------------------------------------------------

using Quit = EmptyEvent;

// ---------------------------------------------------

class Manager
{
protected:
	Mylib::Memory::Manager& memory_manager;
	Mylib::Trigger::EventHandler<KeyDown> event_key_down;
	Mylib::Trigger::EventHandler<TouchScreenMoveData> event_touch_screen_move;
	Mylib::Trigger::EventHandler<Quit> event_quit;
	
public:
	Manager (Mylib::Memory::Manager& memory_manager_)
		: memory_manager(memory_manager_),
		  event_key_down(memory_manager),
		  event_touch_screen_move(memory_manager),
		  event_quit(memory_manager)
	{
	}

	virtual void process_events () = 0;

	Mylib::Trigger::EventHandler<KeyDown>& key_down ()
	{
		return this->event_key_down;
	}

	Mylib::Trigger::EventHandler<TouchScreenMoveData>& touch_screen_move ()
	{
		return this->event_touch_screen_move;
	}

	Mylib::Trigger::EventHandler<Quit>& quit ()
	{
		return this->event_quit;
	}
};

// ---------------------------------------------------

} // end namespace Event
} // end namespace MyGlib

#endif