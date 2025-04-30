#ifndef __MY_GAME_LIB_EXCEPTION_HEADER_H__
#define __MY_GAME_LIB_EXCEPTION_HEADER_H__

#include <utility>

#include <my-lib/exception.h>

#include <boost/static_string.hpp>


// ---------------------------------------------------

namespace MyGlib
{

// ---------------------------------------------------

class Exception : public Mylib::Exception
{
public:
	Exception (const std::source_location& location_, const char *assert_str_)
		: Mylib::Exception(location_, assert_str_)
	{
	}

protected:
	void build_exception_msg (std::ostringstream& str_stream) const override final
	{
		try {
			str_stream << "My Game Lib Exception" << std::endl;
			this->build_mygamelib_exception_msg(str_stream);
		} catch (...) {
			str_stream << "My Game Lib Exception (error building exception message)" << std::endl;
		}
	}

	virtual void build_mygamelib_exception_msg (std::ostringstream& str_stream) const = 0;
};

// ---------------------------------------------------

class TextureNotFoundException : public Exception
{
private:
	// we use a static string to avoid dynamic memory allocation
	boost::static_string<128> texture_id;

public:
	TextureNotFoundException (const std::source_location& location_, const char *assert_str_, const std::string_view texture_id_)
		: Exception(location_, assert_str_), texture_id(texture_id_)
	{
	}

protected:
	void build_mygamelib_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "Texture not found: " << this->texture_id << std::endl;
	}
};

// ---------------------------------------------------

class NoMyGameLibException : public Exception
{
public:
	NoMyGameLibException (const std::source_location& location_, const char *assert_str_)
		: Exception(location_, assert_str_)
	{
	}

protected:
	void build_mygamelib_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "No My Game Lib Exception" << std::endl;
	}
};

// ---------------------------------------------------

class NoMyGameLibAudioException : public Exception
{
public:
	NoMyGameLibAudioException (const std::source_location& location_, const char *assert_str_)
		: Exception(location_, assert_str_)
	{
	}

protected:
	void build_mygamelib_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "No My Game Lib Audio Exception" << std::endl;
	}
};

// ---------------------------------------------------

class NoMyGameLibGraphicsException : public Exception
{
public:
	NoMyGameLibGraphicsException (const std::source_location& location_, const char *assert_str_)
		: Exception(location_, assert_str_)
	{
	}

protected:
	void build_mygamelib_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "No My Game Lib Graphics Exception" << std::endl;
	}
};

// ---------------------------------------------------

class NoMyGameLibEventException : public Exception
{
public:
	NoMyGameLibEventException (const std::source_location& location_, const char *assert_str_)
		: Exception(location_, assert_str_)
	{
	}

protected:
	void build_mygamelib_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "No My Game Lib Event Exception" << std::endl;
	}
};

// ---------------------------------------------------

} // end namespace MyGlib

#endif