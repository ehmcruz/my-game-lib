//#include <variant>

#include <my-game-lib/my-game-lib.h>
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

	SDL_Window *screen = SDL_CreateWindow("My Game Window",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		800, 500,
		SDL_WINDOW_OPENGL);

	SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, 0);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

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

void SDL_EventDriver::check_events ()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				exit(0);
			break;

			case SDL_KEYDOWN:
			break;
		}
	}
}

// ---------------------------------------------------

} // end namespace MyGlib