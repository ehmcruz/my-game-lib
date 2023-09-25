#include <my-game-lib/my-game-lib.h>

bool alive = true;

int main (int argc, char **argv)
{
	MyGlib::Lib& lib = MyGlib::Lib::init();
	MyGlib::EventManager& event_manager = lib.get_event_manager();
	MyGlib::AudioManager& audio_manager = lib.get_audio_manager();

	MyGlib::AudioDescriptor audio_descriptor = audio_manager.load_sound("hq-explosion-6288.wav", MyGlib::AudioFormat::Wav);

	while (alive)
	{
		event_manager.check_events();
	}

	return 0;
}