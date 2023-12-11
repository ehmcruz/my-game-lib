#include <my-game-lib/my-game-lib.h>

#ifdef MYGLIB_SUPPORT_SDL
	#include <my-game-lib/sdl/sdl-driver.h>
	#include <my-game-lib/sdl/sdl-audio.h>
#endif

#ifdef MYGLIB_SUPPORT_OPENGL
	#include <my-game-lib/opengl/opengl.h>
#endif

namespace MyGlib
{

// ---------------------------------------------------

Lib& Lib::init (const InitParams& params)
{
	if (instance == nullptr)
		instance = new Lib(params);
	return *instance;
}

Lib& Lib::init (const InitParams& params, Mylib::Memory::Manager& memory_manager_)
{
	if (instance == nullptr)
		instance = new Lib(params, memory_manager_);
	return *instance;
}

// ---------------------------------------------------

Lib::Lib (const InitParams& params)
	: memory_manager(this->default_memory_manager)
{
	this->lib_init(params);
}

Lib::Lib (const InitParams& params, Mylib::Memory::Manager& memory_manager_)
	: memory_manager(memory_manager_)
{
	this->lib_init(const InitParams& params);
}

void Lib::lib_init (const InitParams& params)
{
#ifdef MYGLIB_SUPPORT_SDL
	SDL_Driver_Init();
	this->audio_manager = new SDL_AudioDriver(this->memory_manager);
	this->event_manager = new SDL_EventDriver(this->memory_manager);
#endif

#ifdef MYGLIB_SUPPORT_OPENGL
	this->graphics_manager = new Opengl::Renderer({
		.memory_manager = this->memory_manager,
		.window_name = params.window_name,
		.window_width_px = params.window_width_px,
		.window_height_px = params.window_height_px,
		.fullscreen = params.fullscreen
	});
#endif

	mylib_assert_exception(this->audio_manager != nullptr)
}

// ---------------------------------------------------

} // end namespace MyGlib