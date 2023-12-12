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

// ---------------------------------------------------

class SDL_AudioDriver: public AudioManager
{
public:
	SDL_AudioDriver (Mylib::Memory::Manager& memory_manager_);
	~SDL_AudioDriver ();
	
public:
	AudioDescriptor load_sound (const std::string_view fname, const AudioFormat format) override final;
	AudioDescriptor load_music (const std::string_view fname, const AudioFormat format) override final;
	void unload_audio (AudioDescriptor& audio) override final;
	void driver_play_audio (AudioDescriptor& audio, Callback *callback) override final;
	void set_volume (AudioDescriptor& audio, const float volume) override final;
};

// ---------------------------------------------------



// ---------------------------------------------------

} // end namespace MyGlib

#endif