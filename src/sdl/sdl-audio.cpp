//#include <variant>

#include <my-game-lib/audio.h>
#include <my-game-lib/sdl/sdl-audio.h>

#include <my-lib/macros.h>
#include <my-lib/trigger.h>

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
	SDL_AudioDescriptor *desc; // (desc == nulptr) means free channel
	AudioManager::Callback *callback;
	size_t callback_size;
};

// ---------------------------------------------------

// we save here because saving in the object would require including the SDL headers in sdl-audio.h
// I want to avoid that

static std::vector<ChannelDescriptor> channels;

// ---------------------------------------------------

inline constexpr Uint16 default_audio_format = MIX_DEFAULT_FORMAT;
inline constexpr int default_audio_channels = 2;
inline constexpr int default_audio_buffers = 4096;
inline constexpr int default_audio_volume = MIX_MAX_VOLUME;
inline constexpr int default_audio_rate = 44100;

// ---------------------------------------------------

static SDL_AudioDriver *driver = nullptr;

// ---------------------------------------------------

SDL_AudioDriver::SDL_AudioDriver (Mylib::Memory::Manager& memory_manager_)
	: AudioManager(memory_manager_)
{
	dprintln("Loading SDL Audio Driver");

	driver = this;

	Uint16 audio_format = default_audio_format;
	int audio_channels = default_audio_channels;
	int audio_buffers = default_audio_buffers;
	int audio_volume = default_audio_volume;
	int audio_rate = default_audio_rate;

	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
		dprintln("Couldn't open audio: " << SDL_GetError());
		exit(1);
	}
	else {
		Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
		dprintln("Opened audio at " << audio_rate << " Hz " << (audio_format&0xFF) << " bit " << audio_channels << " channels " << audio_buffers <<  " bytes audio buffer\n");
	}

	const int nchannels = Mix_AllocateChannels(-1);
	
	dprintln("there are " << nchannels << " channels");
	
	channels.reserve(nchannels);

	for (int32_t i = 0; i < nchannels; i++)
		channels.push_back( ChannelDescriptor {
			.channel = i,
			.desc = nullptr,
			.callback = nullptr,
			.callback_size = 0
			} );

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

	dprintln("loaded sound " << fname);

	return AudioDescriptor { .data = desc };
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
	auto& channel = channels[id];
	SDL_AudioDescriptor *desc = channel.desc;

	dprintln("channel " << id << " finished playing " << desc->fname << ", calling callback");

	auto& c = *(channel.callback);
	
	AudioManager::Event event {
		.type = AudioManager::Event::Type::AudioFinished,
		.audio_descriptor = desc,
		.repeat = false
	};

	c(event);

	mylib_assert_exception(event.repeat == false)

	if (!event.repeat) {
		channel.desc = nullptr;
		driver->get_memory_manager().deallocate(channel.callback, channel.callback_size, 1);
		channel.callback = nullptr;
	}
}

// ---------------------------------------------------

void SDL_AudioDriver::play_audio (AudioDescriptor& audio, Callback *callback, const size_t callback_size)
{
	SDL_AudioDescriptor *desc = audio.data.get_value<SDL_AudioDescriptor*>();

	if (desc->type == SDL_AudioDescriptor::Type::Chunk) {
		const int id = Mix_PlayChannel(-1, desc->chunk, 0);

		dprintln("playing sound " << desc->fname << " at channel " << id);

		auto& channel = channels[id];

		channel.desc = desc;
		channel.callback = callback;
		channel.callback_size = callback_size;

		if (callback)
			Mix_ChannelFinished(sdl_channel_finished_callback);
		else
			Mix_ChannelFinished(nullptr);
	}

	#warning possible race condition when calling callback, will fix later
}

// ---------------------------------------------------

} // end namespace MyGlib