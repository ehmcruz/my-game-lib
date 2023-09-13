#ifndef __MY_GAME_LIB_SOUND_HEADER_H__
#define __MY_GAME_LIB_SOUND_HEADER_H__

#include <string_view>
#include <string_view>

#include <cstdint>

#include <my-lib/macros.h>
#include <my-lib/std.h>
#include <my-lib/trigger.h>
#include <my-lib/any.h>

namespace MyGlib
{

// ---------------------------------------------------

enum SoundFormat {
	Wav,
	MP3
};

struct SoundDescriptor {
	Mylib::Any<sizeof(void*), sizeof(void*)> data;
};

// ---------------------------------------------------

class SoundManager
{
public:
	enum class EventType {
		MusicFinished
	};

	struct Event {
		const EventType type;
		const Sound& sound;
		bool repeat;
	};

	using SoundCallback = Callback<Event>;

private:
	static SoundManager *instance;

private:
	SoundManager () = default;

public:
	SoundManager& init ();

	virtual SoundDescriptor load_sound (const std::string_view fname, const SoundFormat format) = 0;

	template <typename Tcallback>
	virtual void play_background_music (const SoundDescriptor sound, const Tcallback& callback) const = 0;
};

// ---------------------------------------------------

} // end namespace MyGlib

#endif