#ifndef __MY_GAME_LIB_AUDIO_HEADER_H__
#define __MY_GAME_LIB_AUDIO_HEADER_H__

#include <string_view>
#include <string_view>

#include <cstdint>

#include <my-lib/macros.h>
#include <my-lib/std.h>
#include <my-lib/trigger.h>
#include <my-lib/any.h>
#include <my-lib/memory.h>

namespace MyGlib
{

// ---------------------------------------------------

enum class AudioFormat {
	Wav,
	MP3
};

enum class AudioType {
	Sound, // sound effect
	Music
};

struct AudioDescriptor {
//	AudioFormat format;
//	AudioType type;
	Mylib::Any<sizeof(void*), sizeof(void*)> data;
};

// ---------------------------------------------------

class AudioManager
{
public:
	struct Event {
		enum class Type {
			AudioFinished
		};
		const EventType type;
		const AudioDescriptor audio_descriptor;
		bool repeat;
	};

	using Callback = Mylib::Trigger::Callback<Event>;

private:
	static AudioManager *instance;
	Mylib::Memory::DefaultManager default_memory_manager;

protected:
	Mylib::Memory::Manager& memory_manager;

private:
	AudioManager ()
		: memory_manager(this->default_memory_manager)
	{
	}

	AudioManager (Mylib::Memory::Manager& memory_manager_)
		: memory_manager(memory_manager_)
	{
	}

public:
	AudioManager& init ();

	// load sound effect
	virtual AudioDescriptor load_sound (const std::string_view fname, const SoundFormat format) = 0;

	// load background music
	virtual AudioDescriptor load_music (const std::string_view fname, const SoundFormat format) = 0;

	virtual void unload_audio (const AudioDescriptor& audio) = 0;

	inline void play_audio (const AudioDescriptor& audio)
	{
		this->play_audio(audio, nullptr, 0);
	}

	template <typename Tcallback>
	void play_audio (const AudioDescriptor& audio, const Tcallback& callback);

protected:
	virtual void play_audio (const AudioDescriptor& audio, Callback *callback, const size_t callback_size) = 0;
};

// ---------------------------------------------------

template <typename Tcallback>
void AudioManager::play_audio (const AudioDescriptor& audio, const Tcallback& callback)
{
	using Tc = Tcallback;

	Tc *persistent_callback = new (this->memory_manager.allocate( sizeof(Tc) )) Tc(callback);

	this->play_sound(audio, persistent_callback);
}

// ---------------------------------------------------

} // end namespace MyGlib

#endif