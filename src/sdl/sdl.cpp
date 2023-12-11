//#include <variant>

#include <my-game-lib/my-game-lib.h>
#include <my-game-lib/debug.h>
#include <my-game-lib/audio.h>
#include <my-game-lib/sdl/sdl-driver.h>
#include <my-game-lib/sdl/sdl-audio.h>

#include <my-lib/macros.h>
#include <my-lib/trigger.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

namespace MyGlib
{

// ---------------------------------------------------

void SDL_Driver_Init ()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

	dprintln("SDL Driver Initialized");
}

// ---------------------------------------------------

void SDL_Driver_End ()
{
	SDL_Quit();
}

// ---------------------------------------------------

SDL_EventDriver::SDL_EventDriver (Mylib::Memory::Manager& memory_manager_)
	: EventManager(memory_manager_)
{
}

// ---------------------------------------------------

inline Key sdl_key_to_key (const SDL_Keycode sdl_key)
{
	switch (sdl_key) {
		case SDLK_a: return Key::A;
		case SDLK_b: return Key::B;
		case SDLK_c: return Key::C;
		case SDLK_d: return Key::D;
		case SDLK_e: return Key::E;
		case SDLK_f: return Key::F;
		case SDLK_g: return Key::G;
		case SDLK_SPACE: return Key::Space;
		case SDLK_RETURN: return Key::Return;
		default: return Key::Unknown;
	}
}

// ---------------------------------------------------

void SDL_EventDriver::process_events ()
{
	SDL_Event sdl_event;

	while (SDL_PollEvent(&sdl_event)) {
		switch (sdl_event.type) {
			case SDL_QUIT:
				this->event_quit.publish( EventQuit { } );
			break;

			case SDL_KEYDOWN:
				this->event_key_down.publish( EventKeyDown {
					.key = sdl_key_to_key(sdl_event.key.keysym.sym)
				} );
			break;
		}
	}
}

// ---------------------------------------------------

} // end namespace MyGlib