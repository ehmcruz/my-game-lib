#ifndef __MY_GAME_LIB_MAIN_HEADER_H__
#define __MY_GAME_LIB_MAIN_HEADER_H__

#ifndef MYGLIB_SUPPORT_SDL
	#error "SDL support is required"
#endif

#include <string_view>

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
		Graphics::Manager::Type graphics_type;
		std::string_view window_name;
		uint32_t window_width_px;
		uint32_t window_height_px;
		bool fullscreen;
	};

private:
	static inline Lib *instance = nullptr;

	Mylib::Memory::Manager& memory_manager;

	Audio::Manager *audio_manager = nullptr;
	Event::Manager *event_manager = nullptr;
	Graphics::Manager *graphics_manager = nullptr;

private:
	Lib (const InitParams& params);
	Lib (const InitParams& params, Mylib::Memory::Manager& memory_manager_);
	void lib_init (const InitParams& params);
	~Lib ();

public:
	static Lib& init (const InitParams& params);
	static Lib& init (const InitParams& params, Mylib::Memory::Manager& memory_manager_);
	static void quit ();
	
	static Lib& get_instance ()
	{
		mylib_assert_exception(instance != nullptr)
		return *instance;
	}

	Audio::Manager& get_audio_manager ()
	{
		mylib_assert_exception(this->audio_manager != nullptr)
		return *this->audio_manager;
	}

	Event::Manager& get_event_manager ()
	{
		mylib_assert_exception(this->event_manager != nullptr)
		return *this->event_manager;
	}

	Graphics::Manager& get_graphics_manager ()
	{
		mylib_assert_exception(this->graphics_manager != nullptr)
		return *this->graphics_manager;
	}
};

// ---------------------------------------------------

} // end namespace MyGlib

#endif