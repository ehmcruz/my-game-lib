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
	auto& program = *renderer->get_program_triangle_texture();

	const auto transform = this->get_global_transform();
	constexpr uint32_t n_vertices = 6;
	
#if 0
	dprint( "local_pos:" )
	Mylib::Math::println(world_pos);

	dprint( "clip_pos:" )
	Mylib::Math::println(clip_pos);
//exit(1);
#endif

	auto vertices = program.alloc_vertices(n_vertices);
	std::span<Vertex> shape_vertices = rect.get_local_rotated_vertices();

	mylib_assert(shape_vertices.size() == n_vertices)

	for (uint32_t i=0; i<n_vertices; i++) {
		vertices[i].gvertex = shape_vertices[i];
		vertices[i].offset = offset;
		vertices[i].color = color;
	}

	renderer->draw_rect2D(this->rect, this->get_global_position(), this->color);
}

// ---------------------------------------------------

void Sprite2DRenderer::process_render (const float dt)
{
	const auto global_pos = this->get_global_position();
	renderer->draw_rect2D(this->rect, global_pos, { .desc = this->texture });
}

// ---------------------------------------------------

void TileMap::set (const uint32_t row, const uint32_t col, unique_ptr<Sprite2DRenderer> sprite)
{
	auto entity = make_unique<Entity>(Vector::zero(), 0);
	entity->add_child(std::move(sprite));
	this->set(row, col, std::move(entity));
}

void TileMap::set (const uint32_t row, const uint32_t col, TextureDescriptor texture)
{
	auto sprite = make_unique<Sprite2DRenderer>(Vector::zero(), this->tile_size, texture);
	this->set(row, col, std::move(sprite));
}

// ---------------------------------------------------

} // namespace Game
} // namespace MyGlib