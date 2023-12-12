#ifndef __MY_GAME_LIB_AUDIO_SDL_HEADER_H__
#define __MY_GAME_LIB_AUDIO_SDL_HEADER_H__

#include <string_view>
#include <vector>

#include <cstdint>

#include <my-lib/macros.h>
#include <my-lib/std.h>
#include <my-lib/trigger.h>
#include <my-lib/any.h>

#include <my-game-lib/audio.h>

namespace MyGlib
{
namespace Audio
{

// ---------------------------------------------------

class SDL_AudioDriver: public Manager
{
public:
	SDL_AudioDriver (Mylib::Memory::Manager& memory_manager_);
	~SDL_AudioDriver ();
	
public:
	Descriptor load_sound (const std::string_view fname, const Format format) override final;
	Descriptor load_music (const std::string_view fname, const Format format) override final;
	void unload_audio (Descriptor& audio) override final;
	void driver_play_audio (Descriptor& audio, Callback *callback) override final;
	void set_volume (Descriptor& audio, const float volume) override final;
};

// ---------------------------------------------------



// ---------------------------------------------------

} // end namespace Audio
} // end namespace MyGlib

#endif