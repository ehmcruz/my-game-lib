#ifndef __MY_GAME_LIB_MAIN_HEADER_H__
#define __MY_GAME_LIB_MAIN_HEADER_H__

#ifndef MYGLIB_SUPPORT_SDL
	#error "SDL support is required"
#endif

#include <my-lib/memory.h>

#include <my-game-lib/events.h>
#include <my-game-lib/audio.h>
#include <my-game-lib/graphics.h>

namespace MyGlib
{

// ---------------------------------------------------

class Lib
{
public:
	struct InitParams {
		std::string_view window_name,
		uint32_t window_width_px,
		uint32_t window_height_px,
		bool fullscreen;
	};

private:
	static inline Lib *instance = nullptr;

	Mylib::Memory::DefaultManager default_memory_manager;
	Mylib::Memory::Manager& memory_manager;

	AudioManager *audio_manager = nullptr;
	EventManager *event_manager = nullptr;
	GraphicsManager *graphics_manager = nullptr;

private:
	Lib (const InitParams& params);
	Lib (const InitParams& params, Mylib::Memory::Manager& memory_manager_);
	void lib_init (const InitParams& params);

public:
	static Lib& init (const InitParams& params);
	static Lib& init (const InitParams& params, Mylib::Memory::Manager& memory_manager_);
	
	static Lib& get_instance ()
	{
		mylib_assert_exception(instance != nullptr)
		return *instance;
	}

	AudioManager& get_audio_manager ()
	{
		mylib_assert_exception(this->audio_manager != nullptr)
		return *this->audio_manager;
	}

	EventManager& get_event_manager ()
	{
		mylib_assert_exception(this->event_manager != nullptr)
		return *this->event_manager;
	}

	GraphicsManager& get_graphics_manager ()
	{
		mylib_assert_exception(this->graphics_manager != nullptr)
		return *this->graphics_manager;
	}
};

// ---------------------------------------------------

} // end namespace MyGlib

#endif