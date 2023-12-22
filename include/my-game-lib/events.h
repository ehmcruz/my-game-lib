#ifndef __MY_GAME_LIB_EVENTS_HEADER_H__
#define __MY_GAME_LIB_EVENTS_HEADER_H__

#include <string_view>

#include <cstdint>

#include <SDL.h>

#include <my-lib/macros.h>
#include <my-lib/std.h>
#include <my-lib/trigger.h>
#include <my-lib/any.h>
#include <my-lib/memory.h>

// ---------------------------------------------------

namespace MyGlib
{
namespace Event
{

// ---------------------------------------------------

struct EmptyEvent_Data__ {
};

using EmptyEvent = Mylib::Trigger::EventHandler<EmptyEvent_Data__>;

// ---------------------------------------------------

struct KeyDown_Data__ {
	SDL_Keycode key_code;
	SDL_Scancode scan_code;
	Uint16 modifiers;
};

using KeyDown = Mylib::Trigger::EventHandler<KeyDown_Data__>;

// ---------------------------------------------------

struct TouchScreenMove_Data__ {
	#define _MYGLIB_ENUM_CLASS_DIRECTION_VALUES_ \
		_MYGLIB_ENUM_CLASS_DIRECTION_VALUE_(Left) \
		_MYGLIB_ENUM_CLASS_DIRECTION_VALUE_(Right) \
		_MYGLIB_ENUM_CLASS_DIRECTION_VALUE_(Up) \
		_MYGLIB_ENUM_CLASS_DIRECTION_VALUE_(Down)

	enum class Direction : uint8_t {
		#define _MYGLIB_ENUM_CLASS_DIRECTION_VALUE_(V) V,
		_MYGLIB_ENUM_CLASS_DIRECTION_VALUES_
		#undef _MYGLIB_ENUM_CLASS_DIRECTION_VALUE_
	};

	Direction direction;
};

const char* enum_class_to_str (const TouchScreenMove_Data__::Direction value);

inline std::ostream& operator << (std::ostream& out, const TouchScreenMove_Data__::Direction value)
{
	out << enum_class_to_str(value);
	return out;
}

using TouchScreenMove = Mylib::Trigger::EventHandler<TouchScreenMove_Data__>;

// ---------------------------------------------------

using Quit = EmptyEvent;

// ---------------------------------------------------

class Manager
{
protected:
	Mylib::Memory::Manager& memory_manager;
	KeyDown event_key_down;
	TouchScreenMove event_touch_screen_move;
	Quit event_quit;
	Mylib::Trigger::EventHandler<SDL_Event> event_sdl;
	
public:
	Manager (Mylib::Memory::Manager& memory_manager_)
		: memory_manager(memory_manager_),
		  event_key_down(memory_manager),
		  event_touch_screen_move(memory_manager),
		  event_quit(memory_manager)
	{
	}

	virtual ~Manager () = default;

	virtual void process_events () = 0;

	KeyDown& key_down ()
	{
		return this->event_key_down;
	}

	TouchScreenMove& touch_screen_move ()
	{
		return this->event_touch_screen_move;
	}

	Quit& quit ()
	{
		return this->event_quit;
	}

	Mylib::Trigger::EventHandler<SDL_Event>& sdl ()
	{
		return this->event_sdl;
	}
};

// ---------------------------------------------------

} // end namespace Event
} // end namespace MyGlib

#endif