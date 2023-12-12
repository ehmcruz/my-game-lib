#include <ostream>
#include <array>
#include <utility>

#include <my-game-lib/events.h>

namespace MyGlib
{
namespace Event
{

// ---------------------------------------------------

const char* enum_class_to_str (const TouchScreenMoveData::Direction value)
{
	static constexpr auto strs = std::to_array<const char*>({
		#define _MYLIB_ENUM_CLASS_TYPE_VALUE_(V) #V,
		_MYLIB_ENUM_CLASS_TYPE_VALUES_
		#undef _MYLIB_ENUM_CLASS_TYPE_VALUE_
	});

	mylib_assert_exception_msg(std::to_underlying(value) < strs.size(), "invalid enum class value ", std::to_underlying(value))

	return strs[ std::to_underlying(value) ];
}

// ---------------------------------------------------

} // end namespace Event
} // end namespace MyGlib