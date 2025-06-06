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

using Graphics::Opengl::Opengl_TextureDescriptor;

namespace Enums {
	// 6 vertices counter-clockwise
	enum Rect2DVertexPositionIndex {
		RightBottom,
		RightTop,
		LeftTop,
		RightBottomAgain,
		LeftTopAgain,
		LeftBottom
	};
}

// ---------------------------------------------------

static std::array<Vector2, 6> generate_local_vertices_rect2d (const Vector2 size)
{
	const auto hs = size * 0.5f; // half size
	std::array<Vector2, 6> local_vertices;

	using enum Enums::Rect2DVertexPositionIndex;

	// draw counter-clockwise

	// first triangle
	local_vertices[RightBottom].set(hs.x, hs.y);
	local_vertices[RightTop].set(hs.x, -hs.y);
	local_vertices[LeftTop].set(-hs.x, -hs.y);

	// second triangle
	local_vertices[RightBottomAgain].set(hs.x, hs.y);
	local_vertices[LeftTopAgain].set(-hs.x, -hs.y);
	local_vertices[LeftBottom].set(-hs.x, hs.y);

	return local_vertices;
}

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

	std::array<Vector2, n_vertices> local_vertices = generate_local_vertices_rect2d(this->size);

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
	auto *renderer = static_cast<Graphics::Opengl::Renderer*>(Game::renderer);
	auto& program = *renderer->get_program_triangle_texture();

	const Matrix3 transform = this->get_global_transform();
	constexpr uint32_t n_vertices = 6;
	
	using Rect2DVertexPositionIndex = Enums::Rect2DVertexPositionIndex;
	using TextureVertexPositionIndex = Graphics::Enums::TextureVertexPositionIndex;

	std::array<Vector2, n_vertices> local_vertices = generate_local_vertices_rect2d(this->size);
	const Opengl_TextureDescriptor *desc = this->texture.info->data.get_value<Opengl_TextureDescriptor*>();

	auto vertices = program.alloc_vertices(n_vertices);

	for (uint32_t i=0; i<n_vertices; i++) {
		vertices[i].gvertex.pos = transform * Vector3(local_vertices[i], 1);
		vertices[i].gvertex.pos.z = this->z;
		vertices[i].offset.set_zero();
	}

	vertices[Rect2DVertexPositionIndex::RightBottom].tex_coords = Vector3(desc->tex_coords[TextureVertexPositionIndex::RightBottom].x, desc->tex_coords[TextureVertexPositionIndex::RightBottom].y, desc->atlas->texture_depth);
	vertices[Rect2DVertexPositionIndex::RightTop].tex_coords = Vector3(desc->tex_coords[TextureVertexPositionIndex::RightTop].x, desc->tex_coords[TextureVertexPositionIndex::RightTop].y, desc->atlas->texture_depth);
	vertices[Rect2DVertexPositionIndex::LeftTop].tex_coords = Vector3(desc->tex_coords[TextureVertexPositionIndex::LeftTop].x, desc->tex_coords[TextureVertexPositionIndex::LeftTop].y, desc->atlas->texture_depth);
	vertices[Rect2DVertexPositionIndex::RightBottomAgain].tex_coords = Vector3(desc->tex_coords[TextureVertexPositionIndex::RightBottom].x, desc->tex_coords[TextureVertexPositionIndex::RightBottom].y, desc->atlas->texture_depth);
	vertices[Rect2DVertexPositionIndex::LeftTopAgain].tex_coords = Vector3(desc->tex_coords[TextureVertexPositionIndex::LeftTop].x, desc->tex_coords[TextureVertexPositionIndex::LeftTop].y, desc->atlas->texture_depth);
	vertices[Rect2DVertexPositionIndex::LeftBottom].tex_coords = Vector3(desc->tex_coords[TextureVertexPositionIndex::LeftBottom].x, desc->tex_coords[TextureVertexPositionIndex::LeftBottom].y, desc->atlas->texture_depth);
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