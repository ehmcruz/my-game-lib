#ifndef __MY_GAME_LIB_MAIN_HEADER_H__
#define __MY_GAME_LIB_MAIN_HEADER_H__

namespace MyGlib
{

// ---------------------------------------------------

class Lib
{
private:
	static Lib *instance;

private:
	Lib ();

public:
	Lib& Init ();
};

// ---------------------------------------------------

} // end namespace MyGlib

#endif