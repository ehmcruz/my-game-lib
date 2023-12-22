#ifndef __MY_GAME_LIB_AUDIO_HEADER_H__
#define __MY_GAME_LIB_AUDIO_HEADER_H__

#include <string_view>

#include <cstdint>

#include <my-lib/macros.h>
#include <my-lib/std.h>
#include <my-lib/trigger.h>
#include <my-lib/any.h>
#include <my-lib/memory.h>

// ---------------------------------------------------

namespace MyGlib
{
namespace Audio
{

// ---------------------------------------------------

enum class Format {
	Wav,
	MP3
};

enum class Type {
	Sound, // sound effect
	Music
};

struct Descriptor {
//	AudioFormat format;
//	AudioType type;
	uint64_t id;
	Mylib::Any<sizeof(void*), sizeof(void*)> data; // used by backend driver
};

// ---------------------------------------------------

class Manager
{
public:
	struct Event {
		enum class Type {
			AudioFinished
		};
		const Type type;
		const Descriptor audio_descriptor;
		bool repeat;
	};

	using Callback = Mylib::Trigger::Callback<Event>;

private:
	static Manager *instance;

protected:
	Mylib::Memory::Manager& memory_manager;

protected:
	Manager (Mylib::Memory::Manager& memory_manager_)
		: memory_manager(memory_manager_)
	{
	}

public:
	Manager& init ();
	virtual ~Manager () = default;

	Mylib::Memory::Manager& get_memory_manager ()
	{
		return this->memory_manager;
	}

	// load sound effect
	virtual Descriptor load_sound (const std::string_view fname, const Format format) = 0;

	// load background music
	virtual Descriptor load_music (const std::string_view fname, const Format format) = 0;

	virtual void unload_audio (Descriptor& audio) = 0;

	inline void play_audio (Descriptor& audio)
	{
		this->driver_play_audio(audio, nullptr);
	}

	template <typename Tcallback>
	void play_audio (Descriptor& audio, const Tcallback& callback);

	virtual void set_volume (Descriptor& audio, const float volume) = 0;

protected:
	virtual void driver_play_audio (Descriptor& audio, Callback *callback) = 0;
};

// ---------------------------------------------------

template <typename Tcallback>
void Manager::play_audio (Descriptor& audio, const Tcallback& callback)
{
	using Tc = Tcallback;

	Tc *persistent_callback = new (this->memory_manager.allocate( sizeof(Tc), 1 )) Tc(callback);

	this->driver_play_audio(audio, persistent_callback);
}

// ---------------------------------------------------

} // end namespace Audio
} // end namespace MyGlib

#endif