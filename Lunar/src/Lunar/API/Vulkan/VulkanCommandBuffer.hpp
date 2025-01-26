#pragma once

#include "Lunar/Renderer/Renderer.hpp"

#include "Lunar/API/Vulkan/Vulkan.hpp"

namespace Lunar
{

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanCommandBuffer
    ////////////////////////////////////////////////////////////////////////////////////
	class VulkanCommandBuffer /* : public CommandBuffer */
    {
    public:
        // Constructor & Destructor
        VulkanCommandBuffer(RendererID rendererID);
        ~VulkanCommandBuffer();

        // The Begin, End & Submit methods are in the Renderer class.

        // Getters
        inline const VkSemaphore GetVkRenderFinishedSemaphore(uint32_t index) const { return m_RenderFinishedSemaphores[index]; }
        inline const VkFence GetVkInFlightFence(uint32_t index) const { return m_InFlightFences[index]; }
        inline const VkCommandBuffer GetVkCommandBuffer(uint32_t index) const { return m_CommandBuffers[index]; }

    private:
        const RendererID m_RendererID;

        std::vector<VkCommandBuffer> m_CommandBuffers = {};

        // Synchronization objects
        std::vector<VkSemaphore> m_RenderFinishedSemaphores = {};
        std::vector<VkFence> m_InFlightFences = {};
    };

}