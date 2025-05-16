#include <my-game-lib/game/game.h>
#include <my-game-lib/game/components-2d.h>
#include <my-game-lib/graphics.h>


// ---------------------------------------------------

namespace MyGlib
{
namespace Game
{

// ---------------------------------------------------

void Rect2DRenderer::process_render (const float dt)
{
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