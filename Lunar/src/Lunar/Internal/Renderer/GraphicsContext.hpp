#pragma once

#include <cstdint>

#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"

namespace Lunar::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// Selection
	////////////////////////////////////////////////////////////////////////////////////
	template<Info::RenderingAPI API>
	struct ContextSelect;

	template<> struct ContextSelect<Info::RenderingAPI::Vulkan>		{ using Type = typename VulkanContext; };

	////////////////////////////////////////////////////////////////////////////////////
	// GraphicsContext
	////////////////////////////////////////////////////////////////////////////////////
	// Note: You only need one context for an entire application, even with multiple windows
	// So, only initialize it once.
	class GraphicsContext
	{
	public:
		// Getters
		static bool Initialized();

		// Static methods
		static void AttachWindow(void* nativeWindow);

		static void Init();
		static void Destroy();

		// Note: This is an internal function, do not call.
		static ContextSelect<Info::g_RenderingAPI>::Type& GetInternalContext();
	};

}