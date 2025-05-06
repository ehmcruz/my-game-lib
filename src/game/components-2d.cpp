#include <my-game-lib/game/game.h>
#include <my-game-lib/game/components-2d.h>
#include <my-game-lib/graphics.h>


// ---------------------------------------------------

namespace MyGlib
{
namespace Game
{

// ---------------------------------------------------

void GraphicsRect2D::process_render (const float dt)
{
	Graphics::Rect2D rect(this->size.x, this->size.y);
	renderer->draw_rect2D(rect, this->get_global_position(), this->color);
}

// ---------------------------------------------------

} // namespace Game
} // namespace MyGlib