#ifndef __MY_GAME_LIB_MAIN_HEADER_H__
#define __MY_GAME_LIB_MAIN_HEADER_H__

#include <my-lib/memory.h>

#include <my-game-lib/audio.h>

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

private:
	Lib ();
	Lib (Mylib::Memory::Manager& memory_manager_);
	void lib_init ();

public:
	Lib& init ();
};

// ---------------------------------------------------

} // end namespace MyGlib

#endif