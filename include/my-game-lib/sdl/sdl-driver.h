#ifndef __MY_GAME_LIB_SDL_HEADER_H__
#define __MY_GAME_LIB_SDL_HEADER_H__

#include <my-game-lib/my-game-lib.h>

#include <my-lib/memory.h>


namespace MyGlib
{

// ---------------------------------------------------

void SDL_Driver_Init ();
void SDL_Driver_End ();

namespace Event
{
	class SDL_EventDriver : public Manager
	{
	public:
		SDL_EventDriver (Mylib::Memory::Manager& memory_manager_);

		void process_events () override final;
	};
} // end namespace Event

// ---------------------------------------------------

} // end namespace MyGlib

#endif