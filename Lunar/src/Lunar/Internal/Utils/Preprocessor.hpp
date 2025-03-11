#pragma once

namespace Lunar::Internal
{

	// Defines
	#if defined(_MSC_VER)
		#define LU_COMPILER_MSVC
	#elif defined(__GNUC__)
		#define LU_COMPILER_GCC
	#elif defined(__clang__)
		#define LU_COMPILER_CLANG
	#endif

	#if (defined(_WIN32) || defined(_WIN64)) && !defined(LU_PLATFORM_WINDOWS)
		#define LU_PLATFORM_WINDOWS
	#elif defined(__ANDROID__) && !defined(LU_PLATFORM_ANDROID)
		#define LU_PLATFORM_ANDROID
	#elif defined(__APPLE__) && (!defined(LU_PLATFORM_MACOS) || defined(LU_PLATFORM_IOS))
		#include <TargetConditionals.h>
		#if TARGET_OS_MAC
			#define LU_PLATFORM_MACOS
		#elif TARGET_OS_IPHONE
			#define LU_PLATFORM_IOS
		#endif
	#elif defined(__linux__) && !defined(LU_PLATFORM_LINUX)
		#define LU_PLATFORM_LINUX
	#endif

	// Utilities
	#define LU_EXPAND_MACRO(x) x
	#define LU_CONCAT(prefix, suffix) prefix suffix

	#define LU_NUMBER_OF_ARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
	#define LU_NUMBER_OF_ARGS(...) LU_EXPAND_MACRO(LU_NUMBER_OF_ARGS_IMPL(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

	#define LU_EXPAND_MACRO_COUNT_IMPL(macro, count, ...) macro##count(__VA_ARGS__)
	#define LU_EXPAND_MACRO_COUNT(macro, count, ...) LU_EXPAND_MACRO_COUNT_IMPL(macro, count, __VA_ARGS__)

}