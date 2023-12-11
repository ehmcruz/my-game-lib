#ifndef __MY_GAME_LIB_MAIN_HEADER_H__
#define __MY_GAME_LIB_MAIN_HEADER_H__

#include <my-lib/memory.h>

#include <my-game-lib/events.h>
#include <my-game-lib/audio.h>
#include <my-game-lib/graphics.h>

namespace MyGlib
{

// ---------------------------------------------------

class Lib
{
private:
	static Lib *instance;

	Mylib::Memory::DefaultManager default_memory_manager;
	Mylib::Memory::Manager& memory_manager;

	AudioManager *audio_manager = nullptr;
	EventManager *event_manager = nullptr;
	GraphicsManager *graphics_manager = nullptr;

private:
	Lib ();
	Lib (Mylib::Memory::Manager& memory_manager_);
	void lib_init ();

public:
	static Lib& init ();

	AudioManager& get_audio_manager ()
	{
		return *this->audio_manager;
	}

	EventManager& get_event_manager ()
	{
		return *this->event_manager;
	}

	GraphicsManager& get_graphics_manager ()
	{
		return *this->graphics_manager;
	}
};

// ---------------------------------------------------

} // end namespace MyGlib

#endif