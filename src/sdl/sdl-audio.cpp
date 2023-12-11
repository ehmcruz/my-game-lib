//#include <variant>

#include <mutex>
#include <atomic>

#include <my-game-lib/debug.h>
#include <my-game-lib/audio.h>
#include <my-game-lib/sdl/sdl-audio.h>

#include <my-lib/macros.h>
#include <my-lib/trigger.h>
#include <my-lib/std.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

namespace MyGlib
{

// ---------------------------------------------------

struct SDL_AudioDescriptor {
	enum class Type {
		Music,
		Chunk
	};

	Type type;
	AudioFormat format;
	std::string fname;
	Mix_Music *music = nullptr; // use variant later, although I read that variant can't store pointers
	Mix_Chunk *chunk = nullptr;
};

struct ChannelDescriptor {
	int channel;
	bool busy;
	AudioDescriptor audio_descriptor;
	AudioManager::Callback *callback;
};

// ---------------------------------------------------

// we save here because saving in the object would require including the SDL headers in sdl-audio.h
// I want to avoid that

static std::vector<ChannelDescriptor> channels;
static std::mutex channels_mutex;

// ---------------------------------------------------

inline constexpr Uint16 default_audio_format = MIX_DEFAULT_FORMAT;
inline constexpr int default_audio_channels = 2;
inline constexpr int default_audio_buffers = 4096;
inline constexpr int default_audio_volume = MIX_MAX_VOLUME;
inline constexpr int default_audio_rate = 44100;

// ---------------------------------------------------

static SDL_AudioDriver *audio_driver = nullptr;
static std::atomic<uint32_t> next_audio_id = 0;

// ---------------------------------------------------

static void sdl_channel_finished_callback (int id);

// ---------------------------------------------------

SDL_AudioDriver::SDL_AudioDriver (Mylib::Memory::Manager& memory_manager_)
	: AudioManager(memory_manager_)
{
	dprintln("Loading SDL Audio Driver");

	audio_driver = this;

	Uint16 audio_format = default_audio_format;
	int audio_channels = default_audio_channels;
	int audio_buffers = default_audio_buffers;
	int audio_volume = default_audio_volume;
	int audio_rate = default_audio_rate;

	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
		mylib_throw_exception_msg("Couldn't open audio", '\n', SDL_GetError());
	}
	else {
		Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
		dprintln("Opened audio at ",
			audio_rate, " Hz ",
			(audio_format & 0xFF), " bit ",
			audio_channels, " channels ",
			audio_buffers,  " bytes audio buffer",
			'\n');
	}

	const int nchannels = Mix_AllocateChannels(-1);
	
	dprintln("there are ", nchannels, " channels");
	
	channels.reserve(nchannels);

	for (int32_t i = 0; i < nchannels; i++)
		channels.push_back( ChannelDescriptor {
			.channel = i,
			.busy = false,
			.audio_descriptor = { .id = 0, .data = nullptr},
			.callback = nullptr
			} );
	
	Mix_ChannelFinished(sdl_channel_finished_callback);

	dprintln("SDL Audio Driver Loaded");
}

SDL_AudioDriver::~SDL_AudioDriver ()
{
	channels.clear();
}

// ---------------------------------------------------

AudioDescriptor SDL_AudioDriver::load_sound (const std::string_view fname, const AudioFormat format)
{
	SDL_AudioDescriptor *desc = new(this->memory_manager.allocate_type<SDL_AudioDescriptor>(1)) SDL_AudioDescriptor;

	desc->fname = fname;

	switch (format) {
		using enum AudioFormat;

		case Wav:
			desc->type = SDL_AudioDescriptor::Type::Chunk;
			desc->format = Wav;
			desc->chunk = Mix_LoadWAV(fname.data());
		break;

/*		case MP3:
			desc->type = SDL_AudioDescriptor::Type::Music;
			desc->format = MP3;
			desc->music = Mix_LoadMUS(fname_explosion);
		break;*/

		default:
			throw Mylib::Exception("SDL Audio Driver requires sound effects in Wav file format!");
	}

	dprintln("loaded sound ", fname);

	uint32_t id = next_audio_id;
	next_audio_id++;

	return AudioDescriptor {
		.id = id,
		.data = desc
		};
}

// ---------------------------------------------------

void SDL_AudioDriver::unload_audio (AudioDescriptor& audio)
{
	SDL_AudioDescriptor *desc = audio.data.get_value<SDL_AudioDescriptor*>();

	// check if audio not running

	mylib_assert_exception(false)  // not finished implementation

	this->memory_manager.deallocate_type(desc, 1);
}

// ---------------------------------------------------

static void sdl_channel_finished_callback (int id)
{
	channels_mutex.lock();

	auto& channel = channels[id];
	AudioDescriptor audio_descriptor = channel.audio_descriptor;

	SDL_AudioDescriptor *desc = audio_descriptor.data.get_value<SDL_AudioDescriptor*>();

	//dprintln("channel ", id, " finished playing ", desc->fname, ", calling callback");
	
	AudioManager::Event event {
		.type = AudioManager::Event::Type::AudioFinished,
		.audio_descriptor = audio_descriptor,
	};

	if (channel.callback != nullptr) {
		auto& c = *channel.callback;
		c(event);
	}

	// backup so we can deallocate the memory after unlocking the mutex
	auto *ptr = channel.callback;
	auto size = channel.callback->get_size();

	// It would be nice to call the callbback after unlocking the mutex,
	// but we would need to know the callback type to make a copy of it
	// in the stack before unlocking the mutex, but we don't know the type,
	// so it is not possible unless we do some black magic.
	// Since this code should not be critical, let's leave black magic only
	// for places where it will make a performance difference.

	channel.callback = nullptr;
	channel.busy = false;

	channels_mutex.unlock();

	if (ptr != nullptr)
		audio_driver->get_memory_manager().deallocate(ptr, size, 1);
}

// ---------------------------------------------------

void SDL_AudioDriver::driver_play_audio (AudioDescriptor& audio, Callback *callback)
{
	SDL_AudioDescriptor *desc = audio.data.get_value<SDL_AudioDescriptor*>();

	if (desc->type == SDL_AudioDescriptor::Type::Chunk) {
		channels_mutex.lock();

		const int id = Mix_PlayChannel(-1, desc->chunk, 0);

		auto& channel = channels[id];

		channel.busy = true;
		channel.audio_descriptor = audio;
		channel.callback = callback;

		channels_mutex.unlock();

		//dprintln("playing sound ", desc->fname, " at channel ", id);
	}
}

// ---------------------------------------------------

} // end namespace MyGlib