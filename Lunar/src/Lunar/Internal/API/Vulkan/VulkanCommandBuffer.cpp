#include "lupch.h"
#include "VulkanCommandBuffer.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include "Lunar/Internal/Renderer/Renderer.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"

namespace Lunar::Internal
{
    
    ////////////////////////////////////////////////////////////////////////////////////
    // Init & Destroy
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanCommandBuffer::Init(const RendererID renderer)
    {
        VkDevice device = VulkanContext::GetVulkanDevice().GetVkDevice();
        const uint32_t framesInFlight = static_cast<uint32_t>(Renderer::GetRenderer(renderer).GetSpecification().Buffers);
        m_CommandBuffers.resize(framesInFlight);

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = VulkanRenderer::GetRenderer(renderer).GetVulkanSwapChain().GetVkCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

        VK_VERIFY(vkAllocateCommandBuffers(device, &allocInfo, m_CommandBuffers.data()));

        m_RenderFinishedSemaphores.resize(framesInFlight);
        m_InFlightFences.resize(framesInFlight);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < framesInFlight; i++) 
        {
            VK_VERIFY(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]));
            VK_VERIFY(vkCreateFence(device, &fenceInfo, nullptr, &m_InFlightFences[i]));
        }
    }

    void VulkanCommandBuffer::Destroy(const RendererID renderer)
    {
        Renderer::GetRenderer(renderer).Free([rendererID = renderer, commandBuffers = m_CommandBuffers, renderFinishedSemaphores = m_RenderFinishedSemaphores, inFlightFences = m_InFlightFences]()
        {
            VkDevice device = VulkanContext::GetVulkanDevice().GetVkDevice();

            auto& renderer = VulkanRenderer::GetRenderer(rendererID);

            vkFreeCommandBuffers(device, renderer.GetVulkanSwapChain().GetVkCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

            for (size_t i = 0; i < renderFinishedSemaphores.size(); i++) 
            {
                renderer.GetTaskManager().RemoveFromAll(renderFinishedSemaphores[i]);
                renderer.GetTaskManager().RemoveFromAll(inFlightFences[i]);

                vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
                vkDestroyFence(device, inFlightFences[i], nullptr);
            }
        });
    }

}