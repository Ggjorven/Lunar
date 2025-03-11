#pragma once

#include <cstdint>

#include "Lunar/Internal/Utils/Preprocessor.hpp"

namespace Lunar::Internal::Info
{

	////////////////////////////////////////////////////////////////////////////////////
	// Structs
	////////////////////////////////////////////////////////////////////////////////////
	enum class Compiler : uint8_t
	{
		MSVC = 0, 
		GCC, 
		Clang
	};

	enum class Platform : uint8_t
	{
		Windows = 0,
		Linux,
		MacOS,

		// Android,
		// iOS,
	};

	enum class RenderingAPI : uint8_t
	{
		Vulkan = 0,
		// OpenGL,
		// DirectX11,
	};

	////////////////////////////////////////////////////////////////////////////////////
	// Values
	////////////////////////////////////////////////////////////////////////////////////
	#if defined(LU_COMPILER_MSVC)
		inline constexpr const Compiler g_Compiler = Compiler::MSVC;
	#elif defined(LU_COMPILER_GCC)
		inline constexpr const Compiler g_Compiler = Compiler::GCC;
	#elif defined(LU_COMPILER_CLANG)
		inline constexpr const Compiler g_Compiler = Compiler::Clang;
	#else
		#error Lunar Settings: Unsupported compiler.
	#endif

	#if defined(LU_PLATFORM_WINDOWS)
		inline constexpr const Platform g_Platform = Platform::Windows;
	#elif defined(LU_PLATFORM_LINUX) 
		inline constexpr const Platform g_Platform = Platform::Linux;
	#elif defined(LU_PLATFORM_MACOS) 
		inline constexpr const Platform g_Platform = Platform::MacOS;
	#else 
		#error Lunar Settings: Unsupported platform.
	#endif

	inline constexpr const RenderingAPI g_RenderingAPI = RenderingAPI::Vulkan;

}