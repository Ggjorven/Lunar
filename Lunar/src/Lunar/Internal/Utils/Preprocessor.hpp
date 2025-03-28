#pragma once

namespace Lunar::Internal
{

	// Defines
	#if defined(_MSC_VER)
		#define LU_COMPILER_MSVC
	#elif defined(__clang__)
		#define LU_COMPILER_CLANG
	#elif defined(__GNUC__)
		#define LU_COMPILER_GCC
	#endif

	#if (defined(_WIN32) || defined(_WIN64)) && (!defined(LU_PLATFORM_WINDOWS) && !defined(LU_PLATFORM_DESKTOP))
		#define LU_PLATFORM_WINDOWS
		#define LU_PLATFORM_DESKTOP
	#elif defined(__ANDROID__) && (!defined(LU_PLATFORM_ANDROID) && !defined(LU_PLATFORM_MOBILE))
		#define LU_PLATFORM_ANDROID
		#define LU_PLATFORM_MOBILE
	#elif defined(__APPLE__) && ((!defined(LU_PLATFORM_MACOS) || defined(LU_PLATFORM_IOS)) && (!defined(LU_PLATFORM_DESKTOP) || !defined(LU_PLATFORM_MOBILE)))
		#include <TargetConditionals.h>
		#if TARGET_OS_MAC
			#define LU_PLATFORM_MACOS
			#define LU_PLATFORM_DESKTOP
		#elif TARGET_OS_IPHONE
			#define LU_PLATFORM_IOS
			#define LU_PLATFORM_MOBILE
		#endif
	#elif defined(__linux__) && (!defined(LU_PLATFORM_LINUX) && !defined(LU_PLATFORM_DESKTOP))
		#define LU_PLATFORM_LINUX
		#define LU_PLATFORM_DESKTOP
	#endif

	#if defined(__ANDROID__) || defined(__linux__) || defined(__APPLE__)
		#define LU_PLATFORM_UNIX
	#endif

	#if defined(__APPLE__)
		#define LU_PLATFORM_APPLE
	#endif

	#define LU_CPPSTD_11 11
	#define LU_CPPSTD_14 14
	#define LU_CPPSTD_17 17
	#define LU_CPPSTD_20 20
	#define LU_CPPSTD_23 23
	#define LU_CPPSTD_26 26
	#define LU_CPPSTD_LATEST LU_CPPSTD_26

	#if defined(LU_COMPILER_MSVC)
		#if _MSVC_LANG >= 202602L
			#define LU_CPPSTD LU_CPPSTD_26
		#elif _MSVC_LANG >= 202302L
			#define LU_CPPSTD LU_CPPSTD_23
		#elif _MSVC_LANG >= 202002L
			#define LU_CPPSTD LU_CPPSTD_20
		#elif _MSVC_LANG >= 201703L
			#define LU_CPPSTD LU_CPPSTD_17
		#elif _MSVC_LANG >= 201402L
			#define LU_CPPSTD LU_CPPSTD_14
		#elif _MSVC_LANG >= 201103L
			#define LU_CPPSTD LU_CPPSTD_11
		#else
			#error Lunar Preprocessor: Unable to query C++ standard.
		#endif
	#else
		#if __cplusplus == 202602L
			#define LU_CPPSTD LU_CPPSTD_26
		#elif __cplusplus >= 202302L
			#define LU_CPPSTD LU_CPPSTD_23
		#elif __cplusplus >= 202002L
			#define LU_CPPSTD LU_CPPSTD_20
		#elif __cplusplus >= 201703L
			#define LU_CPPSTD LU_CPPSTD_17
		#elif __cplusplus >= 201402L
			#define LU_CPPSTD LU_CPPSTD_14
		#elif __cplusplus >= 201103L
			#define LU_CPPSTD LU_CPPSTD_11
		#else
			#error Lunar Preprocessor: Unable to query C++ standard.
		#endif
	#endif

	// Utilities
	#define LU_EXPAND_MACRO(x) x
	#define LU_CONCAT(prefix, suffix) prefix suffix

	#define LU_NUMBER_OF_ARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
	#define LU_NUMBER_OF_ARGS(...) LU_EXPAND_MACRO(LU_NUMBER_OF_ARGS_IMPL(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

	#define LU_EXPAND_MACRO_COUNT_IMPL(macro, count, ...) macro##count(__VA_ARGS__)
	#define LU_EXPAND_MACRO_COUNT(macro, count, ...) LU_EXPAND_MACRO_COUNT_IMPL(macro, count, __VA_ARGS__)

}