#pragma once

#include <string>

#include "Lunar/Utils/Preprocessor.hpp"

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