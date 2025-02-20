#include <chrono>
#include <thread>
#include <string_view>

#include <my-game-lib/my-game-lib.h>

#include <my-lib/event.h>

using MyGlib::Graphics::Vector;
using MyGlib::Graphics::Point;
using MyGlib::Graphics::Quaternion;
using MyGlib::Graphics::Vector2;
using MyGlib::Graphics::Vector4f;
using MyGlib::Graphics::fp_t;
using MyGlib::Graphics::fp;
using MyGlib::Graphics::Color;
using MyGlib::Graphics::LightPointDescriptor;
using MyGlib::Graphics::Cube3D;
using MyGlib::Graphics::Sphere3D;
using MyGlib::Graphics::Rect2D;
using MyGlib::Graphics::Circle2D;
using MyGlib::Graphics::Line3D;
using MyGlib::Graphics::WireCube3D;
using MyGlib::Graphics::TextureDescriptor;

using Colors = MyGlib::Graphics::Colors;

using Clock = std::chrono::steady_clock;
using ClockDuration = Clock::duration;
using ClockTime = Clock::time_point;

constexpr ClockDuration fp_to_ClockDuration (const fp_t t)
{
	return std::chrono::duration_cast<ClockDuration>(std::chrono::duration<fp_t>(t));
}

constexpr fp_t ClockDuration_to_fp (const ClockDuration& d)
{
	return std::chrono::duration_cast<std::chrono::duration<fp_t>>(d).count();
}

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

void key_down_callback (const MyGlib::Event::KeyDown::Type& event)
{
	switch (event.key_code)
	{
		case SDLK_SPACE: {
			std::cout << "Playing audio " << audio_explosion.id << std::endl;
			audio_manager->play_audio(audio_explosion, Mylib::Event::make_callback_function<MyGlib::Audio::Manager::Event>(&my_sound_callback));
			}
			break;
	
		case SDLK_RETURN:
			std::cout << "Playing audio " << audio_explosion.id << " without callback" << std::endl;
			audio_manager->play_audio(audio_explosion);
			break;
		
		case SDLK_ESCAPE:
			alive = false;
		break;
	
		default:
			break;
	}
}

LightPointDescriptor light;

Cube3D cube (1);
Vector cube_pos (-2, -2, 4);

Cube3D cube_color (1.5);

Cube3D far_cube (40);
fp_t far_cube_angular_vel = Mylib::Math::degrees_to_radians(fp(45));

Sphere3D sphere(2);
Sphere3D earth(2);

Point camera_pos(-0.5, -0.5, 10);
Point camera_vector(0, 0, -1);

Rect2D samus_rect;

Color ambient_light_color {1, 1, 1, 0.3};

TextureDescriptor chrono_texture;
TextureDescriptor earth_high_texture;
TextureDescriptor earth_medium_texture;
TextureDescriptor earth_low_texture;
TextureDescriptor flowers_texture;
TextureDescriptor fzero_texture;
TextureDescriptor house_texture;
TextureDescriptor mario_texture;
TextureDescriptor moon_high_texture;
TextureDescriptor moon_medium_texture;
TextureDescriptor moon_low_texture;
TextureDescriptor samus_texture;
TextureDescriptor half_samus_texture;
TextureDescriptor tree_texture;
TextureDescriptor yoshi_texture;
TextureDescriptor zelda_texture;
TextureDescriptor box_texture;

void setup ()
{
	light = renderer->add_light_point_source(
		Point(-10, 10, -10), Colors::white
	);

	std::cout << "Light id: " << light << std::endl;

	cube.rotate(Vector(0, 1, 0), 0);
	earth.rotate(Vector(0, 1, 0), 0);
	far_cube.rotate(Vector(0.2, 1, 0.3), 0);

	renderer->begin_texture_loading();
	chrono_texture = renderer->load_texture("tests-assets/chrono.png");
	earth_high_texture = renderer->load_texture("tests-assets/earth-high.jpg");
	earth_medium_texture = renderer->load_texture("tests-assets/earth-medium.jpg");
	earth_low_texture = renderer->load_texture("tests-assets/earth-low.jpg");
	flowers_texture = renderer->load_texture("tests-assets/flowers.jpg");
	fzero_texture = renderer->load_texture("tests-assets/fzero.png");
	house_texture = renderer->load_texture("tests-assets/house.png");
	mario_texture = renderer->load_texture("tests-assets/mario.png");
	moon_high_texture = renderer->load_texture("tests-assets/moon-high.jpg");
	moon_medium_texture = renderer->load_texture("tests-assets/moon-medium.jpg");
	moon_low_texture = renderer->load_texture("tests-assets/moon-low.jpg");
	
	renderer->load_texture("samus", "tests-assets/samus.png");
	samus_texture = renderer->find_texture("samus");
	
	tree_texture = renderer->load_texture("tests-assets/tree.png");
	yoshi_texture = renderer->load_texture("tests-assets/yoshi.jpg");
	zelda_texture = renderer->load_texture("tests-assets/zelda.jpg");
	box_texture = renderer->load_texture("tests-assets/box.png");
	renderer->end_texture_loading();

	half_samus_texture = renderer->create_sub_texture(samus_texture, 0, 0, samus_texture.info->width_px / 2, samus_texture.info->height_px);
	
	samus_rect.set_size(1.0, 1.0 / samus_texture.info->aspect_ratio);
}

static void process_keys (const Uint8 *keys, const fp_t dt)
{
	constexpr fp_t speed = 1.0;
	constexpr fp_t rot_speed = Mylib::Math::degrees_to_radians(fp(90));

	if (keys[SDL_SCANCODE_UP])
		cube_pos.y += speed * dt;
	else if (keys[SDL_SCANCODE_DOWN])
		cube_pos.y -= speed * dt;

	if (keys[SDL_SCANCODE_LEFT])
		cube_pos.x -= speed * dt;
	else if (keys[SDL_SCANCODE_RIGHT])
		cube_pos.x += speed * dt;

	if (keys[SDL_SCANCODE_PERIOD])
		cube_pos.z -= speed * dt;
	else if (keys[SDL_SCANCODE_COMMA])
		cube_pos.z += speed * dt;
	
	if (keys[SDL_SCANCODE_N]) {
		cube.rotate(cube.get_rotation_angle() + rot_speed * dt);
		earth.rotate(earth.get_rotation_angle() + rot_speed * dt);
	}
	else if (keys[SDL_SCANCODE_M]) {
		cube.rotate(cube.get_rotation_angle() - rot_speed * dt);
		earth.rotate(earth.get_rotation_angle() - rot_speed * dt);
	}
}

void update (const fp_t dt)
{
	process_keys(SDL_GetKeyboardState(NULL), dt);

	far_cube.rotate(far_cube.get_rotation_angle() + far_cube_angular_vel * dt);
}

void render ()
{
	constexpr float zoom = 1.0;
	renderer->wait_next_frame();

#if 0
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

	renderer->draw_rect2D(Rect2D(4, 2), Vector(3, 3, z_2d), Color::red());
	renderer->draw_rect2D(Rect2D(6, 3), Vector(5, 5, z_2d-0.05), { .desc = earth_high_texture });
	renderer->draw_circle2D(Circle2D(3), Vector(5, 5, z_2d), Color::green());
	renderer->draw_circle2D(Circle2D(0.5), Vector(8, 8, z_2d), Color::blue());
	renderer->draw_rect2D(samus_rect, Vector(6, 3, z_2d-0.1), { .desc = samus_texture });

	renderer->render();

	renderer->clear_buffers(MyGlib::Graphics::Manager::VertexBufferBit | MyGlib::Graphics::Manager::DepthBufferBit);
#else
	renderer->setup_render_3D( MyGlib::Graphics::RenderArgs3D {
		.world_camera_pos = camera_pos,
		.world_camera_target = camera_pos + camera_vector,
		.world_camera_up = Vector(0, 1, 0),
		.projection = MyGlib::Graphics::PerspectiveProjectionInfo {
				.fov_y = Mylib::Math::degrees_to_radians(fp(45)),
				.z_near = 100,
				.z_far = 100000,
			},
		.ambient_light_color = ambient_light_color,
		} );
	
	const Vector far_cube_pos = Vector(2, -3, -100);
	const Color far_cube_color = Colors::yellow;

	renderer->draw_cube3D(far_cube, far_cube_pos, far_cube_color);

	renderer->render();

	#if 1
		renderer->clear_buffers(MyGlib::Graphics::Manager::VertexBufferBit | MyGlib::Graphics::Manager::DepthBufferBit);

		renderer->setup_render_3D( MyGlib::Graphics::RenderArgs3D {
			.world_camera_pos = camera_pos,
			.world_camera_target = camera_pos + camera_vector,
			.world_camera_up = Vector(0, 1, 0),
			.projection = MyGlib::Graphics::PerspectiveProjectionInfo {
				.fov_y = Mylib::Math::degrees_to_radians(fp(45)),
				.z_near = 0.1,
				.z_far = 100,
			},
			.ambient_light_color = ambient_light_color,
			} );
		
		renderer->draw_cube3D(cube, cube_pos, { .desc = box_texture });
		renderer->draw_cube3D(cube_color, Vector(3, -3, -1), Colors::red);
		renderer->draw_wire_cube3D(WireCube3D(1), cube_pos+Vector(2, 0, 0), Colors::white);
		renderer->draw_sphere3D(sphere, Vector(2.5, 1.5, 0), Colors::green);
		renderer->draw_sphere3D(earth, Vector(-4, 0, 0), { .desc = earth_medium_texture });

		// we render the far cube twice because part of it is
		// in the first frustum and part of it is in the second frustum
		renderer->draw_cube3D(far_cube, far_cube_pos, far_cube_color);

		renderer->draw_line3D(Line3D(Vector(5, 0, 1)), Vector(-1, 0, 2), Colors::blue);

		auto rect_samus = Rect2D(1, 1);
		rect_samus.set_scale_y(-1);
		rect_samus.calculate_vertices();

		Quaternion q = Quaternion::rotation(Vector(1, 0, 0), Vector(0, 1, 0));
		auto [axis, angle] = q.to_axis_angle();
		rect_samus.rotate(axis, angle);

		auto rect_half_samus = Rect2D(1, 1);
		rect_half_samus.set_scale_y(-1);

		renderer->draw_rect2D(rect_samus, Vector(-1, 0, 1), { .desc = samus_texture });
		renderer->draw_rect2D(rect_half_samus, Vector(0, 0, 1), { .desc = half_samus_texture });

		renderer->render();
	#endif
#endif

	renderer->update_screen();
}

void quit_callback (const MyGlib::Event::Quit::Type& event)
{
	alive = false;
}

int main (int argc, char **argv)
{
	MyGlib::Graphics::Manager::Type graphics_type;

	if (argc == 2) {
		if (std::string_view(argv[1]) == "opengl")
			graphics_type = MyGlib::Graphics::Manager::Type::Opengl;
		else if (std::string_view(argv[1]) == "sdl")
			graphics_type = MyGlib::Graphics::Manager::Type::SDL;
		else {
			std::cout << "Invalid graphics type: " << argv[1] << std::endl;
			return 1;
		}
	}
	else {
		std::cout << "Usage: " << argv[0] << " [opengl|sdl]" << std::endl;
		return 1;
	}

	lib = &MyGlib::Lib::init({
		.graphics_type = graphics_type,
		//.graphics_type = MyGlib::Graphics::Manager::Type::Opengl,
		.window_name = "My Game Lib Test",
		.window_width_px = 1200,
		.window_height_px = 800,
		//.fullscreen = true
		.fullscreen = false
	});
	event_manager = &lib->get_event_manager();
	audio_manager = &lib->get_audio_manager();
	renderer = &lib->get_graphics_manager();

	setup();

	music = audio_manager->load_music("tests-assets/music.mp3", MyGlib::Audio::Format::MP3);
	audio_explosion = audio_manager->load_sound("tests-assets/hq-explosion-6288.wav", MyGlib::Audio::Format::Wav);

	audio_manager->play_audio(music, Mylib::Event::make_callback_function<MyGlib::Audio::Manager::Event>(&my_music_callback));

	event_manager->key_down().subscribe( Mylib::Event::make_callback_function<MyGlib::Event::KeyDown::Type>(&key_down_callback) );
	event_manager->quit().subscribe( Mylib::Event::make_callback_function<MyGlib::Event::Quit::Type>(&quit_callback) );

	constexpr fp_t dt = 1.0 / 60.0;
	int frame = 0;

	while (alive)
	{
		//std::cout << "rendering frame " << frame << std::endl;

		update(dt);
		event_manager->process_events();
		render();

		std::this_thread::sleep_for(fp_to_ClockDuration(dt));
		frame++;
	}

	return 0;
}