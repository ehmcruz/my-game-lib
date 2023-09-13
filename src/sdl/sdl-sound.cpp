#include <my-game-lib/sound.h>
#include <my-game-lib/sdl/sdl-sound.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

namespace MyGlib
{

// ---------------------------------------------------

struct SDL_SoundDescriptor {
	enum class Type {
		Music,
		Chunk
	};

	Type type;
	SoundFormat format;
	Mix_Music *music = nullptr;
	Mix_Chunk *chunk = nullptr;
};

// ---------------------------------------------------

SoundDescriptor SDL_SoundDriver::load_sound (const std::string_view fname, const SoundFormat format)
{
	auto *desc = new SDL_SoundDescriptor;

	switch (format) {
		using enum SoundFormat;

		case Wav:
			desc->type = SDL_SoundDescriptor::Type::Chunk;
			desc->format = Wav;
			desc->chunk = Mix_LoadWAV(fname_explosion);
		break;

		case MP3:
			desc->type = SDL_SoundDescriptor::Type::Music;
			desc->format = Music;
			desc->music = Mix_LoadMUS(fname_explosion);
		break;

		default:
	}

	return SoundDescriptor { .data = Any(desc) };
}

// ---------------------------------------------------

template <typename Tcallback>
void play_background_music (const SoundDescriptor sound, const Tcallback& callback) const
{

}

// ---------------------------------------------------

} // end namespace MyGlib