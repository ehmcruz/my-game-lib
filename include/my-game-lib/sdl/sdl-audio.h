#ifndef __MY_GAME_LIB_SDL_AUDIO_HEADER_H__
#define __MY_GAME_LIB_SDL_AUDIO_HEADER_H__

#include <string_view>
#include <vector>

#include <cstdint>

#include <my-lib/macros.h>
#include <my-lib/std.h>
#include <my-lib/trigger.h>
#include <my-lib/any.h>

#include <my-game-lib/sound.h>

namespace MyGlib
{

// ---------------------------------------------------

class SDL_AudioDriver: public AudioManager
{
private:
	SDL_AudioDriver ();
	SDL_AudioDriver (Mylib::Memory::Manager& memory_manager_);
	~SDL_AudioDriver ();
	void init ();

public:
	AudioDescriptor load_sound (const std::string_view fname, const SoundFormat format) override;
	void unload_audio (const AudioDescriptor& audio) override;
	void play_audio (const AudioDescriptor& audio, Callback *callback, const size_t callback_size) override;
};

// ---------------------------------------------------



// ---------------------------------------------------

} // end namespace MyGlib

#endif