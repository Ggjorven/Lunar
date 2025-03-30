#pragma once

#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanCommandBuffer.hpp"

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Selection
    ////////////////////////////////////////////////////////////////////////////////////
    template<Info::RenderingAPI API>
    struct CommandBufferSelect;

    template<> struct CommandBufferSelect<Info::RenderingAPI::Vulkan> { using Type = VulkanCommandBuffer; };

    using CommandBufferType = typename CommandBufferSelect<Info::g_RenderingAPI>::Type;

    ////////////////////////////////////////////////////////////////////////////////////
	// CommandBuffer
    ////////////////////////////////////////////////////////////////////////////////////
    class CommandBuffer
    {
    public:
        // Constructor & Destructor
		inline CommandBuffer(const RendererID rendererID) { m_CommandBuffer.Init(rendererID); }
        inline ~CommandBuffer() { m_CommandBuffer.Destroy(); }

        // The Begin, End & Submit methods are in the Renderer class.
        
		// Getters
		inline RendererID GetRendererID() const { return m_CommandBuffer.GetRendererID(); }

        // Internal methods
		inline CommandBufferType& GetInternalCommandBuffer() { return m_CommandBuffer; }

    private:
        CommandBufferType m_CommandBuffer = {};
    };

}