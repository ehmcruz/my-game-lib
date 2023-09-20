#include <my-game-lib/my-game-lib.h>

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
	this->audio_manager = new SDL_AudioDriver(this->memory_manager);
#endif

	mylib_assert_exception(this->audio_manager != nullptr)
}

// ---------------------------------------------------

} // end namespace MyGlib

#endif