#include <ostream>
#include <array>
#include <utility>

#include <my-game-lib/events.h>
#include <my-game-lib/exception.h>

#include <my-lib/std.h>


namespace MyGlib
{
namespace Event
{

// ---------------------------------------------------

const char* enum_class_to_str (const TouchScreenMove_Data__::Direction value)
{
	static constexpr auto strs = std::to_array<const char*>({
		#define _MYGLIB_ENUM_CLASS_DIRECTION_VALUE_(V) #V,
		_MYGLIB_ENUM_CLASS_DIRECTION_VALUES_
		#undef _MYGLIB_ENUM_CLASS_DIRECTION_VALUE_
	});

	using EnumType = typename Mylib::remove_type_qualifiers<decltype(value)>::type;
	using ExceptionType = typename Mylib::InvalidEnumClassValueException<EnumType>;
	//using ExceptionType = typename Mylib::InvalidEnumClassValueException<TouchScreenMove_Data__::Direction>;

	mylib_assert_exception_args(std::to_underlying(value) < strs.size(), ExceptionType, value)

	return strs[ std::to_underlying(value) ];
}

// ---------------------------------------------------

} // end namespace Event
} // end namespace MyGlib