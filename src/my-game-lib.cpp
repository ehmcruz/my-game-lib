#include <my-game-lib/my-game-lib.h>

namespace MyGlib
{

// ---------------------------------------------------

Lib *Lib::instance = nullptr;

// ---------------------------------------------------

Lib& Lib::Init ()
{
	if (Lib::instance == nullptr)
		Lib::instance = new Lib;
	return * Lib::instance;
}

// ---------------------------------------------------

} // end namespace MyGlib

#endif