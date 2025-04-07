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
		inline CommandBuffer() = default;
		inline CommandBuffer(const RendererID renderer) { Init(renderer); }
		inline ~CommandBuffer() = default;

        // Init & Destroy
		inline void Init(const RendererID renderer) { m_CommandBuffer.Init(renderer); }
        inline void Destroy(const RendererID renderer) { m_CommandBuffer.Destroy(renderer); }

        // The Begin, End & Submit methods are in the Renderer class.
        
        // Internal methods
		inline CommandBufferType& GetInternalCommandBuffer() { return m_CommandBuffer; }

    private:
        CommandBufferType m_CommandBuffer = {};
    };

}