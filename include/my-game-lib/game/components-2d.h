#ifndef __MY_GAME_LIB_GAME_COMPONENTS2D_HEADER_H__
#define __MY_GAME_LIB_GAME_COMPONENTS2D_HEADER_H__

#include <my-game-lib/game/game.h>


// ---------------------------------------------------

namespace MyGlib
{
namespace Game
{

// ---------------------------------------------------

class Rect2DRenderer : public TransformComponent2D, public RenderInterface
{
public:
	using TransformComponent = TransformComponent2D;
	using Vector = TransformComponent::Vector;
	using Point = TransformComponent::Point;

private:
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(Graphics::Rect2D, rect)
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(Color, color)

public:
	Rect2DRenderer (const Point& position_, const Vector& size_, const Color& color_)
		: TransformComponent2D(position_),
		  rect(size_),
		  color(color_)
	{
	}

	void process_render (const float dt) override final;
};

// ---------------------------------------------------

class Sprite2DRenderer : public TransformComponent2D, public RenderInterface
{
public:
	using TransformComponent = TransformComponent2D;
	using Vector = TransformComponent::Vector;
	using Point = TransformComponent::Point;

private:
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(Graphics::Rect2D, rect)
	MYLIB_OO_ENCAPSULATE_OBJ_WITH_COPY_MOVE(TextureDescriptor, texture)

public:
	Sprite2DRenderer (const Point& position_, const Vector& size_, const TextureDescriptor& texture_)
		: TransformComponent2D(position_),
		  rect(size_),
		  texture(texture_)
	{
	}

	void process_render (const float dt) override final;
};

// ---------------------------------------------------

} // end namespace Game
} // end namespace MyGlib

#endif