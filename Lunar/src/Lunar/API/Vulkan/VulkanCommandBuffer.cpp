#include "lupch.h"
#include "VulkanCommandBuffer.hpp"

#include "Lunar/IO/Print.hpp"

#include "Lunar/API/Vulkan/VulkanContext.hpp"
#include "Lunar/API/Vulkan/VulkanRenderer.hpp"

namespace Lunar
{
    
    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanCommandBuffer::VulkanCommandBuffer(RendererID rendererID) 
        : m_RendererID(rendererID)
    {
        VkDevice device = VulkanContext::GetVulkanDevice()->GetVkDevice();
        const uint32_t framesInFlight = (uint32_t)VulkanRenderer::GetRenderer(m_RendererID).GetSpecification().Buffers;
        m_CommandBuffers.resize(framesInFlight);

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = VulkanRenderer::GetRenderer(m_RendererID).GetVulkanSwapChain()->GetVkCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

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

    VulkanCommandBuffer::~VulkanCommandBuffer()
    {
        VulkanRenderer::GetRenderer(m_RendererID).Free([rendererID = m_RendererID, commandBuffers = m_CommandBuffers, renderFinishedSemaphores = m_RenderFinishedSemaphores, inFlightFences = m_InFlightFences]() 
            {
                VkDevice device = VulkanContext::GetVulkanDevice()->GetVkDevice();

                auto& renderer = VulkanRenderer::GetRenderer(rendererID);

                vkFreeCommandBuffers(VulkanContext::GetVulkanDevice()->GetVkDevice(), renderer.GetVulkanSwapChain()->GetVkCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

                for (size_t i = 0; i < renderFinishedSemaphores.size(); i++) 
                {
                    // Note: In obscure ways this sometimes gets called after the renderer is destroyed.
                    // I don't want to impose some kind of lifetime rules, so this is the solution.
                    if (&renderer != nullptr) 
                    {
                        renderer.GetTaskManager().RemoveFromAll(renderFinishedSemaphores[i]);
                        renderer.GetTaskManager().RemoveFromAll(inFlightFences[i]);
                    }

                    vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
                    vkDestroyFence(device, inFlightFences[i], nullptr);
                }
            });
    }

}