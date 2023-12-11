#include <my-game-lib/my-game-lib.h>

#include <my-lib/trigger.h>

bool alive = true;

MyGlib::Lib *lib;
MyGlib::EventManager *event_manager;
MyGlib::AudioManager *audio_manager;

MyGlib::AudioDescriptor audio_explosion;

void my_audio_callback (MyGlib::AudioManager::Event& event)
{
	std::cout << "Audio " << event.audio_descriptor.id << " finished" << std::endl;
}

void key_down_callback (const MyGlib::EventKeyDown& event)
{
	switch (event.key)
	{
		case MyGlib::Key::Space: {
			std::cout << "Playing audio " << audio_explosion.id << std::endl;
			audio_manager->play_audio(audio_explosion, Mylib::Trigger::make_callback_function<MyGlib::AudioManager::Event>(&my_audio_callback));
			}
			break;
	
		case MyGlib::Key::Return:
			alive = false;
			break;
	
		default:
			break;
	}
}

void quit_callback (const MyGlib::EventQuit& event)
{
	alive = false;
}

int main (int argc, char **argv)
{
	lib = &MyGlib::Lib::init({
		.window_name = "My Game Lib Test",
		.window_width_px = 640,
		.window_height_px = 480,
		.fullscreen = false
	});
	event_manager = &lib->get_event_manager();
	audio_manager = &lib->get_audio_manager();

	audio_explosion = audio_manager->load_sound("hq-explosion-6288.wav", MyGlib::AudioFormat::Wav);

	event_manager->key_down().subscribe( Mylib::Trigger::make_callback_function<MyGlib::EventKeyDown>(&key_down_callback) );
	event_manager->quit().subscribe( Mylib::Trigger::make_callback_function<MyGlib::EventQuit>(&quit_callback) );

	while (alive)
	{
		event_manager->process_events();
	}

	return 0;
}