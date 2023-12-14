#include <my-game-lib/my-game-lib.h>

#include <my-lib/trigger.h>

using MyGlib::Graphics::Vector;
using MyGlib::Graphics::Point;
using MyGlib::Graphics::Vector2;
using MyGlib::Graphics::fp_t;
using MyGlib::Graphics::fp;

bool alive = true;

MyGlib::Lib *lib;
MyGlib::Event::Manager *event_manager;
MyGlib::Audio::Manager *audio_manager;
MyGlib::Graphics::Manager *renderer;

MyGlib::Audio::Descriptor audio_explosion;
MyGlib::Audio::Descriptor music;

void my_sound_callback (MyGlib::Audio::Manager::Event& event)
{
	std::cout << "Sound effect " << event.audio_descriptor.id << " finished" << std::endl;
}

void my_music_callback (MyGlib::Audio::Manager::Event& event)
{
	std::cout << "Music " << event.audio_descriptor.id << " finished" << std::endl;
}

void key_down_callback (const MyGlib::Event::KeyDown& event)
{
	switch (event.key_code)
	{
		case SDLK_SPACE: {
			std::cout << "Playing audio " << audio_explosion.id << std::endl;
			audio_manager->play_audio(audio_explosion, Mylib::Trigger::make_callback_function<MyGlib::Audio::Manager::Event>(&my_sound_callback));
			}
			break;
	
		case SDLK_RETURN:
			std::cout << "Playing audio " << audio_explosion.id << " without callback" << std::endl;
			audio_manager->play_audio(audio_explosion);
			break;
	
		default:
			break;
	}
}

void render ()
{
	constexpr float zoom = 1.0;
	renderer->wait_next_frame();

#if 1
	const Vector2 ws = renderer->get_normalized_window_size();

	renderer->setup_render_2D( MyGlib::Graphics::RenderArgs2D {
		.clip_init_norm = Vector2(0, 0),
		.clip_end_norm = Vector2(ws.x, ws.y),
		.world_init = Vector2(0, 0),
		.world_end = Vector2(10, 10),
		.force_camera_inside_world = true,
		.world_camera_focus = Vector2(5, 5),
		.world_screen_width = 10 * (1.0f / zoom)
		} );

	const fp_t z_2d = 0.98;

	renderer->draw_rect2D(MyGlib::Graphics::Rect2D(4, 2), Vector(3, 3, z_2d), MyGlib::Graphics::Color::red());
	renderer->draw_circle2D(MyGlib::Graphics::Circle2D(3), Vector(5, 5, z_2d), MyGlib::Graphics::Color::green());
	renderer->draw_circle2D(MyGlib::Graphics::Circle2D(0.5), Vector(8, 8, z_2d), MyGlib::Graphics::Color::blue());

	renderer->render();

	renderer->clear_vertex_buffers();
#endif

	MyGlib::Graphics::Cube3D cube (1);
	Vector cube_pos (-1, -1, -4);

	renderer->setup_render_3D( MyGlib::Graphics::RenderArgs3D {
		.world_camera_pos = Point(0, 0, 0),
		.world_camera_target = Point(0, 0, -1),
		.fov_y = Mylib::Math::degrees_to_radians(fp(45)),
		.z_near = 0.1,
		.z_far = 10
		} );
	
	renderer->draw_cube3D(cube, cube_pos, MyGlib::Graphics::Color::red());

	renderer->render();

	renderer->update_screen();
}

void quit_callback (const MyGlib::Event::Quit& event)
{
	alive = false;
}

int main (int argc, char **argv)
{
	lib = &MyGlib::Lib::init({
		.window_name = "My Game Lib Test",
		.window_width_px = 1200,
		.window_height_px = 800,
		.fullscreen = false
	});
	event_manager = &lib->get_event_manager();
	audio_manager = &lib->get_audio_manager();
	renderer = &lib->get_graphics_manager();

	music = audio_manager->load_music("music.mp3", MyGlib::Audio::Format::MP3);
	audio_explosion = audio_manager->load_sound("hq-explosion-6288.wav", MyGlib::Audio::Format::Wav);

	audio_manager->play_audio(music, Mylib::Trigger::make_callback_function<MyGlib::Audio::Manager::Event>(&my_music_callback));

	event_manager->key_down().subscribe( Mylib::Trigger::make_callback_function<MyGlib::Event::KeyDown>(&key_down_callback) );
	event_manager->quit().subscribe( Mylib::Trigger::make_callback_function<MyGlib::Event::Quit>(&quit_callback) );

	while (alive)
	{
		event_manager->process_events();
		render();
	}

	return 0;
}