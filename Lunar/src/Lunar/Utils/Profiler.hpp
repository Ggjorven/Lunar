#pragma once

#include <string>

// Note: For future profiling 
namespace Lunar
{

	// Function name
	#if defined(_MSC_VER)
		#define LU_PRETTY_FUNCTION __FUNCSIG__
	#elif defined(__GNUC__) || defined(__clang__)
		#define LU_PRETTY_FUNCTION __PRETTY_FUNCTION__
	#else 
		#define LU_PRETTY_FUNCTION __func__ 
	#endif

	// Utilities
	#define LU_EXPAND_MACRO(x) x
	#define LU_CONCAT(prefix, suffix) prefix suffix

	#define LU_NUMBER_OF_ARGS(...) LU_EXPAND_MACRO(LU_NUMBER_OF_ARGS_IMPL(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
	#define LU_NUMBER_OF_ARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N

	#define LU_EXPAND_MACRO_COUNT(macro, count, ...) LU_EXPAND_MACRO_COUNT_IMPL(macro, count, __VA_ARGS__)
	#define LU_EXPAND_MACRO_COUNT_IMPL(macro, count, ...) macro##count(__VA_ARGS__)

	// Function naming (if count = 1, class was passed in) // TODO: Fix the __VA_ARGS__ expansion since it always says a class is passed in because of ','
	#define LU_FUNC_NAME(...) LU_FUNC_NAME_IMPL(__VA_ARGS__)

	#define LU_FUNC_NAME_IMPL(...) LU_EXPAND_MACRO_COUNT(LU_EXPAND_FUNC_NAME_, LU_NUMBER_OF_ARGS(__VA_ARGS__), __VA_ARGS__)

	#define LU_EXPAND_FUNC_NAME_0(fn) fn
	#define LU_EXPAND_FUNC_NAME_1(fn, cls) cls + "::" + std::string(__func__)

	// Profiling macros
	#if !defined(LU_CONFIG_DIST)
		#define LU_MARK_FRAME()
		
		#define LU_PROFILE(name)
		
		// Note: Optional class argument
		#define LU_PROFILE_SCOPE(...)
	#else
		#define LU_MARK_FRAME()

		#define LU_PROFILE(name)

		// Note: Optional class argument
		#define LU_PROFILE_SCOPE(...)
	#endif

}