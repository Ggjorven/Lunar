#pragma once

#include "Lunar/Internal/API/Vulkan/Vulkan.hpp"

#include "Lunar/Internal/Renderer/RendererSpec.hpp"

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanCommandBuffer
    ////////////////////////////////////////////////////////////////////////////////////
	class VulkanCommandBuffer
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