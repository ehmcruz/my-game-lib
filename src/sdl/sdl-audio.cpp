#include <variant>
#include <mutex>
#include <atomic>

#include <my-game-lib/debug.h>
#include <my-game-lib/audio.h>
#include <my-game-lib/sdl/sdl-driver.h>
#include <my-game-lib/exception.h>

#include <my-lib/macros.h>
#include <my-lib/event.h>
#include <my-lib/std.h>

#include <SDL.h>
#include <SDL_mixer.h>

namespace MyGlib
{
namespace Audio
{

// ---------------------------------------------------

struct SDL_AudioDescriptor {
	enum class Type {
		Music,
		Chunk
	};

	Type type;
	Format format;
	float volume;
	std::string fname;
	std::variant<Mix_Music*, Mix_Chunk*> ptr;
};

struct ChannelDescriptor {
	int id;
	bool busy;
	Descriptor audio_descriptor;
	Mylib::Memory::unique_ptr<Manager::Callback> callback;
};

// ---------------------------------------------------

// we save here because saving in the object would require including the SDL headers in sdl-audio.h
// I want to avoid that

static std::vector<ChannelDescriptor> channels;
static std::mutex channels_mutex;

static ChannelDescriptor music_channel;
static std::mutex music_channel_mutex;

// ---------------------------------------------------

static constexpr Uint16 default_audio_format = MIX_DEFAULT_FORMAT;
static constexpr int default_audio_channels = 2;
static constexpr int default_audio_buffers = 4096;
static constexpr int default_audio_volume = MIX_MAX_VOLUME;
static constexpr int default_audio_rate = 44100;

// ---------------------------------------------------

static SDL_AudioDriver *audio_driver = nullptr;
static std::atomic<uint64_t> next_audio_id {0};

// ---------------------------------------------------

static void sdl_channel_finished_callback (int id);
static void sdl_music_finished_callback ();

// ---------------------------------------------------

SDL_AudioDriver::SDL_AudioDriver (Mylib::Memory::Manager& memory_manager_)
	: Manager(memory_manager_)
{
	dprintln("Loading SDL Audio Driver");

	audio_driver = this;

	Uint16 audio_format = default_audio_format;
	int audio_channels = default_audio_channels;
	int audio_buffers = default_audio_buffers;
	int audio_volume = default_audio_volume;
	int audio_rate = default_audio_rate;

	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
		mylib_throw_msg(NoMyGameLibAudioException, "Couldn't open audio driver");
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
			.id = i,
			.busy = false,
			.audio_descriptor = { .id = 0, .data = nullptr},
			.callback = nullptr
			} );
	
	music_channel = ChannelDescriptor {
		.id = -1,
		.busy = false,
		.audio_descriptor = { .id = 0, .data = nullptr},
		.callback = nullptr
		};
	
	Mix_ChannelFinished(sdl_channel_finished_callback);
	Mix_HookMusicFinished(sdl_music_finished_callback);

	dprintln("SDL Audio Driver Loaded");
}

SDL_AudioDriver::~SDL_AudioDriver ()
{
	channels.clear();
}

// ---------------------------------------------------

Descriptor SDL_AudioDriver::load_sound (const std::string_view fname, const Format format)
{
	SDL_AudioDescriptor *desc = new(this->memory_manager.allocate_type<SDL_AudioDescriptor>(1)) SDL_AudioDescriptor;

	desc->fname = fname;

	switch (format) {
		using enum Format;

		case Wav:
			desc->type = SDL_AudioDescriptor::Type::Chunk;
			desc->format = Wav;
			desc->ptr = Mix_LoadWAV(fname.data());

			mylib_assert_exception_args(std::get<Mix_Chunk*>(desc->ptr) != nullptr, UnableToLoadAudioException, fname)
		break;

		default:
			mylib_throw_msg_args(UnableToLoadAudioException, "SDL Audio Driver requires sound effects in Wav file format!", fname);
	}

	desc->volume = 1.0f;

	dprintln("loaded sound ", fname);

	return Descriptor {
		.id = next_audio_id++,
		.data = desc
		};
}

Descriptor SDL_AudioDriver::load_music (const std::string_view fname, const Format format)
{
	SDL_AudioDescriptor *desc = new(this->memory_manager.allocate_type<SDL_AudioDescriptor>(1)) SDL_AudioDescriptor;
//music = Mix_LoadMUS(fname);
	desc->fname = fname;

	switch (format) {
		using enum Format;

		case Wav:
			desc->type = SDL_AudioDescriptor::Type::Chunk;
			desc->format = Wav;
			desc->ptr = Mix_LoadWAV(fname.data());

			mylib_assert_exception_args(std::get<Mix_Chunk*>(desc->ptr) != nullptr, UnableToLoadAudioException, fname)
		break;

		case MP3:
			desc->type = SDL_AudioDescriptor::Type::Music;
			desc->format = MP3;
			desc->ptr = Mix_LoadMUS(fname.data());

			mylib_assert_exception_args(std::get<Mix_Music*>(desc->ptr) != nullptr, UnableToLoadAudioException, fname)
		break;

		default:
			mylib_throw_msg_args(UnableToLoadAudioException, "SDL Audio Driver: unsupported audio format for music.", fname);
	}

	desc->volume = 1.0f;

	dprintln("loaded music ", fname);

	return Descriptor {
		.id = next_audio_id++,
		.data = desc
		};
}

// ---------------------------------------------------

void SDL_AudioDriver::unload_audio (Descriptor& audio)
{
	SDL_AudioDescriptor *desc = audio.data.get_value<SDL_AudioDescriptor*>();

	// check if audio not running

	mylib_assert(false)  // not finished implementation

	this->memory_manager.deallocate_type(desc, 1);
}

// ---------------------------------------------------

static void call_callback (Descriptor audio_descriptor, Mylib::Memory::unique_ptr<Manager::Callback> ptr_callback)
{
	if (ptr_callback) {
		SDL_AudioDescriptor *desc = audio_descriptor.data.get_value<SDL_AudioDescriptor*>();

		Manager::Event event {
			.type = Manager::Event::Type::AudioFinished,
			.audio_descriptor = audio_descriptor,
		};

		auto& c = *ptr_callback;
		c(event);

		// ptr_callback will automatically be deallocated when going out of scope
	}
}

// ---------------------------------------------------

static void sdl_channel_finished_callback (int id)
{
	auto& channel = channels[id];

	channels_mutex.lock();

	// backup so we can:
	// - call the callback after unlocking the mutex
	// - deallocate the memory after unlocking the mutex
	auto ptr_callback = std::move(channel.callback);
	Descriptor audio_descriptor = channel.audio_descriptor;
	channel.busy = false;

	channels_mutex.unlock();

	//dprintln("channel ", id, " finished playing ", desc->fname, ", calling callback");

	call_callback(audio_descriptor, std::move(ptr_callback));
}

// ---------------------------------------------------

static void sdl_music_finished_callback ()
{
	music_channel_mutex.lock();

	// backup so we can:
	// - call the callback after unlocking the mutex
	// - deallocate the memory after unlocking the mutex
	auto ptr_callback = std::move(music_channel.callback);
	Descriptor audio_descriptor = music_channel.audio_descriptor;
	music_channel.busy = false;

	music_channel_mutex.unlock();

	//dprintln("channel ", id, " finished playing ", desc->fname, ", calling callback");

	call_callback(audio_descriptor, std::move(ptr_callback));
}

// ---------------------------------------------------

void SDL_AudioDriver::driver_play_audio (Descriptor& audio, Mylib::Memory::unique_ptr<Callback> callback)
{
	SDL_AudioDescriptor *desc = audio.data.get_value<SDL_AudioDescriptor*>();

	if (desc->type == SDL_AudioDescriptor::Type::Chunk) {
		channels_mutex.lock();

		ChannelDescriptor *channel = nullptr;

		for (auto& c: channels) {
			if (!c.busy) {
				channel = &c;
				break;
			}
		}

		mylib_assert_exception_msg_args(channel != nullptr, UnableToPlayAudioException, "SDL Audio Driver: no free channel available to play sound effect", desc->fname)

		const int channel_playing = Mix_PlayChannel(channel->id, std::get<Mix_Chunk*>(desc->ptr), 0);

		mylib_assert_exception_msg_args(channel_playing == channel->id, UnableToPlayAudioException, "SDL Audio Driver: something really bad happenned.", desc->fname)

		channel->busy = true;
		channel->audio_descriptor = audio;
		channel->callback = std::move(callback);

		channels_mutex.unlock();

		//dprintln("playing sound ", desc->fname, " at channel ", channel->id);
	}
	else { // music
		music_channel_mutex.lock();

		Mix_VolumeMusic(static_cast<int>(desc->volume * static_cast<float>(MIX_MAX_VOLUME)));
		Mix_PlayMusic(std::get<Mix_Music*>(desc->ptr), 0);

		music_channel.busy = true;
		music_channel.audio_descriptor = audio;
		music_channel.callback = std::move(callback);

		music_channel_mutex.unlock();
	}
}

// ---------------------------------------------------

void SDL_AudioDriver::set_volume (Descriptor& audio, const float volume)
{
	SDL_AudioDescriptor *desc = audio.data.get_value<SDL_AudioDescriptor*>();

	desc->volume = volume;

	if (desc->type == SDL_AudioDescriptor::Type::Chunk) {
		Mix_VolumeChunk(std::get<Mix_Chunk*>(desc->ptr), static_cast<int>(volume * static_cast<float>(MIX_MAX_VOLUME)));
	}
	else { // music

	}
}

// ---------------------------------------------------

} // end namespace Audio
} // end namespace MyGlib