#pragma once

#include "Lunar/Internal/API/Vulkan/Vulkan.hpp"

#include "Lunar/Internal/Renderer/RendererSpec.hpp"

namespace Lunar::Internal
{

    class VulkanRenderer;

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanCommandBuffer
    ////////////////////////////////////////////////////////////////////////////////////
	class VulkanCommandBuffer
    {
    public:
        // Constructor & Destructor
        VulkanCommandBuffer() = default;
        ~VulkanCommandBuffer() = default;

        // Init & Destroy
		void Init(RendererID rendererID);
        void Destroy();

        // The Begin, End & Submit methods are in the Renderer class.

        // Getters
        inline VkSemaphore GetVkRenderFinishedSemaphore(uint32_t index) const { return m_RenderFinishedSemaphores[index]; }
        inline VkFence GetVkInFlightFence(uint32_t index) const { return m_InFlightFences[index]; }
        inline VkCommandBuffer GetVkCommandBuffer(uint32_t index) const { return m_CommandBuffers[index]; }

		inline RendererID GetRendererID() const { return m_RendererID; }

    private:
        RendererID m_RendererID = {};
        std::vector<VkCommandBuffer> m_CommandBuffers = {};

        // Synchronization objects
        std::vector<VkSemaphore> m_RenderFinishedSemaphores = {};
        std::vector<VkFence> m_InFlightFences = {};

        friend class VulkanRenderer;
    };

}