#ifndef __MY_GAME_LIB_EXCEPTION_HEADER_H__
#define __MY_GAME_LIB_EXCEPTION_HEADER_H__

#include <utility>

#include <my-lib/exception.h>

#include <boost/static_string.hpp>


// ---------------------------------------------------

namespace MyGlib
{

// ---------------------------------------------------

inline constexpr std::size_t fname_max_length = 128;

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

class LightLimitException : public Exception
{
public:
	LightLimitException (const std::source_location& location_, const char *assert_str_)
		: Exception(location_, assert_str_)
	{
	}

protected:
	void build_mygamelib_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "Light limit exception";
	}
};

// ---------------------------------------------------

class TextureNotFoundException : public Exception
{
private:
	// we use a static string to avoid dynamic memory allocation
	boost::static_string<fname_max_length> texture_id;

public:
	TextureNotFoundException (const std::source_location& location_, const char *assert_str_, const std::string_view texture_id_)
		: Exception(location_, assert_str_), texture_id(texture_id_)
	{
	}

protected:
	void build_mygamelib_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "Texture not found: \"" << this->texture_id << "\".";
	}
};

// ---------------------------------------------------

class UnableToAddTextureException : public Exception
{
private:
	// we use a static string to avoid dynamic memory allocation
	boost::static_string<fname_max_length> texture_id;

public:
	UnableToAddTextureException (const std::source_location& location_, const char *assert_str_, const std::string_view texture_id_)
		: Exception(location_, assert_str_), texture_id(texture_id_)
	{
	}

protected:
	void build_mygamelib_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "Unable to add texture \"" << this->texture_id << "\"." << std::endl;
	}
};

// ---------------------------------------------------

class SplitTextureNotDivisibleException : public Exception
{
private:
	// we use a static string to avoid dynamic memory allocation
	boost::static_string<fname_max_length> texture_id;
	uint32_t n_rows;
	uint32_t n_cols;
	uint32_t width_px;
	uint32_t height_px;

public:
	SplitTextureNotDivisibleException (const std::source_location& location_,
	                                   const char *assert_str_,
									   const std::string_view texture_id_,
									   const uint32_t n_rows_,
									   const uint32_t n_cols_,
									   const uint32_t width_px_,
									   const uint32_t height_px_)
		: Exception(location_, assert_str_), texture_id(texture_id_),
		  n_rows(n_rows_), n_cols(n_cols_), width_px(width_px_), height_px(height_px_)
	{
	}

protected:
	void build_mygamelib_exception_msg (std::ostringstream& str_stream) const override final
	{
		str_stream << "Split texture not divisible exception." << std::endl
			<< "Texture id: \"" << this->texture_id << "\"." << std::endl
			<< "n_rows: " << this->n_rows << std::endl
			<< "n_cols: " << this->n_cols << std::endl
			<< "width_px: " << this->width_px << std::endl
			<< "height_px: " << this->height_px << std::endl;
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