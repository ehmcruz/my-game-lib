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
	uint32_t id;
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
		const Type type;
		const AudioDescriptor audio_descriptor;
		bool repeat;
	};

	using Callback = Mylib::Trigger::Callback<Event>;

private:
	static AudioManager *instance;

protected:
	Mylib::Memory::Manager& memory_manager;

protected:
	AudioManager (Mylib::Memory::Manager& memory_manager_)
		: memory_manager(memory_manager_)
	{
	}

public:
	AudioManager& init ();

	Mylib::Memory::Manager& get_memory_manager ()
	{
		return this->memory_manager;
	}

	// load sound effect
	virtual AudioDescriptor load_sound (const std::string_view fname, const AudioFormat format) = 0;

	// load background music
	//virtual AudioDescriptor load_music (const std::string_view fname, const AudioFormat format) = 0;

	virtual void unload_audio (AudioDescriptor& audio) = 0;

	inline void play_audio (AudioDescriptor& audio)
	{
		this->play_audio(audio, nullptr, 0);
	}

	template <typename Tcallback>
	void play_audio (AudioDescriptor& audio, const Tcallback& callback);

protected:
	virtual void play_audio (AudioDescriptor& audio, Callback *callback, const size_t callback_size) = 0;
};

// ---------------------------------------------------

template <typename Tcallback>
void AudioManager::play_audio (AudioDescriptor& audio, const Tcallback& callback)
{
	using Tc = Tcallback;

	Tc *persistent_callback = new (this->memory_manager.allocate( sizeof(Tc), 1 )) Tc(callback);

	this->play_audio(audio, persistent_callback, sizeof(Tc));
}

// ---------------------------------------------------

} // end namespace MyGlib

#endif