//#include <variant>

#include <my-game-lib/sound.h>
#include <my-game-lib/sdl/sdl-sound.h>
#include <my-lib/macros.h>

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
	Callback *callback;
	size_t callback_size;
};

// ---------------------------------------------------

// we save here because saving in the object would require including the SDL headers in sdl-audio.h
// I want to avoid that

static std::vector<SDL_AudioDescriptor> channels;

// ---------------------------------------------------

SDL_AudioDriver (Mylib::Memory::Manager& memory_manager_)
	: AudioManager(memory_manager_)
{
	this->init();
}

SDL_AudioDriver::~SDL_AudioDriver ()
{
	channels.clear();
}

void SDL_AudioDriver::init ()
{
	dprintln("Loading SDL Audio Driver");

	const int nchannels = Mix_AllocateChannels(-1);
	
	dprintln("there are " << nchannels << " channels");
	
	channels.reserve(nchannels);

	for (uint32_t i = 0; i < nchannels; i++)
		channels.push_back( ChannelDescriptor {
			.channel = i,
			.desc = nullptr
			} );
}

// ---------------------------------------------------

AudioDescriptor SDL_AudioDriver::load_sound (const std::string_view fname, const SoundFormat format)
{
	SDL_AudioDescriptor *desc = this->memory_manager.allocate_type<SDL_AudioDescriptor>(1);

	desc->fname = fname;

	switch (format) {
		using enum AudioFormat;

		case Wav:
			desc->type = SDL_AudioDescriptor::Type::Chunk;
			desc->format = Wav;
			desc->chunk = Mix_LoadWAV(fname_explosion);
		break;

/*		case MP3:
			desc->type = SDL_AudioDescriptor::Type::Music;
			desc->format = MP3;
			desc->music = Mix_LoadMUS(fname_explosion);
		break;*/

		default:
			throw Mylib::Exception("SDL Audio Driver requires sound effects in Wav file format!");
	}

	return AudioDescriptor { .data = desc };
}

// ---------------------------------------------------

void SDL_AudioDriver::unload_audio (const AudioDescriptor& audio)
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
	
	Event event {
		.type = Event::Type::AudioFinished,
		.audio_descriptor = ,
		.repeat = false
	};

	c(event);

	mylib_assert_exception(event.repeat == false)

	if (!event.repeat) {
		channel.desc = nullptr;
		this->memory_manager.deallocate(channel.callback, channel.callback_size);
		channel.callback = nullptr;
	}
}

// ---------------------------------------------------

void SDL_AudioDriver::play_audio (const AudioDescriptor& audio, Callback *callback, const size_t callback_size)
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