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
	renderer->draw_rect2D(this->rect, this->get_global_position(), { .desc = this->texture });
}

// ---------------------------------------------------

} // namespace Game
} // namespace MyGlib