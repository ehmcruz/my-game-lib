#include <my-game-lib/my-game-lib.h>

#ifdef MYGLIB_SUPPORT_SDL
#include <my-game-lib/sdl/sdl-driver.h>
#include <my-game-lib/sdl/sdl-audio.h>
#endif

namespace MyGlib
{

// ---------------------------------------------------

Lib *Lib::instance = nullptr;

// ---------------------------------------------------

Lib& Lib::init ()
{
	if (Lib::instance == nullptr)
		Lib::instance = new Lib;
	return *Lib::instance;
}

// ---------------------------------------------------

Lib::Lib ()
	: memory_manager(this->default_memory_manager)
{
	this->lib_init();
}

Lib::Lib (Mylib::Memory::Manager& memory_manager_)
	: memory_manager(memory_manager_)
{
	this->lib_init();
}

void Lib::lib_init ()
{
#ifdef MYGLIB_SUPPORT_SDL
	SDL_Driver_Init();
	this->audio_manager = new SDL_AudioDriver(this->memory_manager);
	this->event_manager = new SDL_EventDriver(this->memory_manager);
#endif

	mylib_assert_exception(this->audio_manager != nullptr)
}

// ---------------------------------------------------

} // end namespace MyGlib