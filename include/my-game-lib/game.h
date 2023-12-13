#ifndef __MY_GAME_LIB_GAME_HEADER_H__
#define __MY_GAME_LIB_GAME_HEADER_H__

#ifdef __MINGW32__
	#define SDL_MAIN_HANDLED
#endif

#include <my-game-lib/graphics.h>

#include <my-lib/std.h>
#include <my-lib/macros.h>

// ---------------------------------------------------

namespace MyGlib
{
namespace Game
{

// ---------------------------------------------------

using Graphics::Vector;

// ---------------------------------------------------

template <typename T>
class Shape : public T
{
private:
	OO_ENCAPSULATE_OBJ_INIT(Vector, pos, Vector::zero())
public:
};

using Cube3D = Shape<Graphics::Cube3D>;

// ---------------------------------------------------

class Body
{
public:
	enum class Type : uint32_t {
		Static,
		Rigid
	};

private:
	OO_ENCAPSULATE_SCALAR_CONST_READONLY(Type, type)

public:
	Body (const Type type_) : type(type_) {}
	virtual ~Body () = default;
};

// ---------------------------------------------------

class StaticBody : public Body
{
private:
	OO_ENCAPSULATE_OBJ_INIT(Vector, pos, Vector::zero())

public:
	StaticBody () : Body(Type::Static) {}
};

// ---------------------------------------------------

class RigidBody : public StaticBody
{
private:
	OO_ENCAPSULATE_OBJ_INIT(Vector, vel, Vector::zero())
	OO_ENCAPSULATE_OBJ_INIT(Vector, acc, Vector::zero())

};

// ---------------------------------------------------

} // end namespace Game
} // end namespace MyGlib

#endif