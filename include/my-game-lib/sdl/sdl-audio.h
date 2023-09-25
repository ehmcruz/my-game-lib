#ifndef __MY_GAME_LIB_SDL_AUDIO_HEADER_H__
#define __MY_GAME_LIB_SDL_AUDIO_HEADER_H__

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
	AudioDescriptor load_sound (const std::string_view fname, const AudioFormat format) override;
	void unload_audio (AudioDescriptor& audio) override;
	void play_audio (AudioDescriptor& audio, Callback *callback, const size_t callback_size) override;
};

// ---------------------------------------------------



// ---------------------------------------------------

} // end namespace MyGlib

#endif