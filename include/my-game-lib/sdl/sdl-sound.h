#ifndef __MY_GAME_LIB_SDL_SOUND_HEADER_H__
#define __MY_GAME_LIB_SDL_SOUND_HEADER_H__

#include <string_view>
#include <string_view>

#include <cstdint>

#include <my-lib/macros.h>
#include <my-lib/std.h>
#include <my-lib/trigger.h>
#include <my-lib/any.h>

#include <my-game-lib/sound.h>

namespace MyGlib
{

// ---------------------------------------------------

class SDL_SoundDriver: public SoundManager
{
private:
	SDL_SoundDriver ();

public:
	virtual SoundDescriptor load_sound (const std::string_view fname, const SoundFormat format) override;

	template <typename Tcallback>
	virtual void play_background_music (const SoundDescriptor sound, const Tcallback& callback) const override;
};

// ---------------------------------------------------

template <typename Tcallback>
virtual void SDL_SoundDriver::play_background_music (const SoundDescriptor sound, const Tcallback& callback) const
{

}

// ---------------------------------------------------

} // end namespace MyGlib

#endif