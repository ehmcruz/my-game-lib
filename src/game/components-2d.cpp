#include <my-game-lib/game/game.h>
#include <my-game-lib/game/components-2d.h>
#include <my-game-lib/graphics.h>
#include <my-game-lib/opengl/opengl.h>


// ---------------------------------------------------

namespace MyGlib
{
namespace Game
{

// ---------------------------------------------------

void Rect2DRenderer::process_render (const float dt)
{
	auto *renderer = static_cast<Graphics::Opengl::Renderer*>(Game::renderer);
	auto& program = *renderer->get_program_triangle_color();

	const Matrix3 transform = this->get_global_transform();
	constexpr uint32_t n_vertices = 6;
	
#if 0
	dprint( "local_pos:" )
	Mylib::Math::println(world_pos);

	dprint( "clip_pos:" )
	Mylib::Math::println(clip_pos);
//exit(1);
#endif

	std::array<Vector2, n_vertices> local_vertices;
	const auto hs = this->size * 0.5f; // half size

	// draw counter-clockwise

	// first triangle
	local_vertices[0] = Point2(hs.x, hs.y); // right bottom
	local_vertices[1] = Point2(hs.x, -hs.y); // right top
	local_vertices[2] = Point2(-hs.x, -hs.y); // left top

	// second triangle
	local_vertices[3] = Point2(hs.x, hs.y); // right bottom
	local_vertices[4] = Point2(-hs.x, -hs.y); // left top
	local_vertices[5] = Point2(-hs.x, hs.y); // left bottom

	auto vertices = program.alloc_vertices(n_vertices);

	for (uint32_t i=0; i<n_vertices; i++) {
		vertices[i].gvertex.pos = transform * Vector3(local_vertices[i], 1);
		vertices[i].gvertex.pos.z = this->z;
		vertices[i].offset.set_zero();
		vertices[i].color = this->color;
	}
}

// ---------------------------------------------------

void Sprite2DRenderer::process_render (const float dt)
{
	//const auto global_pos = this->get_global_position();
	//renderer->draw_rect2D(this->rect, global_pos, { .desc = this->texture });
}

// ---------------------------------------------------

void TileMap::set (const uint32_t row, const uint32_t col, unique_ptr<Sprite2DRenderer> sprite)
{
	auto entity = make_unique<Entity>(0);
	entity->add_child(std::move(sprite));
	this->set(row, col, std::move(entity));
}

void TileMap::set (const uint32_t row, const uint32_t col, TextureDescriptor texture)
{
	auto sprite = make_unique<Sprite2DRenderer>(this->tile_size, texture);
	this->set(row, col, std::move(sprite));
}

// ---------------------------------------------------

} // namespace Game
} // namespace MyGlib