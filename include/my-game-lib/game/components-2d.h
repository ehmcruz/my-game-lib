#ifndef __MY_GAME_LIB_GAME_COMPONENTS2D_HEADER_H__
#define __MY_GAME_LIB_GAME_COMPONENTS2D_HEADER_H__

#include <my-game-lib/game/game.h>


// ---------------------------------------------------

namespace MyGlib
{
namespace Game
{

// ---------------------------------------------------

class GraphicsRect2D : public SpatialComponent2D
{
public:
	using SpatialComponent = SpatialComponent2D;
	using Vector = SpatialComponent::Vector;
	using Point = SpatialComponent::Point;

private:
	MYLIB_OO_ENCAPSULATE_OBJ(Vector, size)
	MYLIB_OO_ENCAPSULATE_OBJ(Color, color)

public:
	GraphicsRect2D (Entity *parent_, SpatialComponent *anchor_, const Point& position_, const Vector& size_, const Color& color_)
		: Component(parent_),
		  SpatialComponent2D(parent_, anchor_, position_),
		  size(size_),
		  color(color_)
	{
	}

	void process_render (const float dt) override final;
};

// ---------------------------------------------------

} // end namespace Game
} // end namespace MyGlib

#endif