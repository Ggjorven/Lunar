#include "lupch.h"
#include "VulkanRenderer.hpp"

#include "Lunar/Internal/IO/Print.hpp"
#include "Lunar/Internal/Utils/Profiler.hpp"
#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/Core/Window.hpp"

#include "Lunar/Internal/Renderer/Image.hpp"
#include "Lunar/Internal/Renderer/Renderpass.hpp"
#include "Lunar/Internal/Renderer/CommandBuffer.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanImage.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanRenderpass.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanCommandBuffer.hpp"

#include <array>
#include <numeric>

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::Init(const RendererSpecification& specs, RendererID id) 
    {
		m_ID = id;
		m_Specification = specs;
        m_TaskManager.Init(m_ID, static_cast<uint32_t>(specs.Buffers));

        m_SwapChain.Init(specs.WindowRef);
    }

    void VulkanRenderer::Destroy()
    {
		// Wait for the device to finish
        {
            VulkanContext::GetVulkanDevice().Wait();
		    vkQueueWaitIdle(VulkanContext::GetVulkanDevice().GetQueue(Queue::Graphics));
		    vkQueueWaitIdle(VulkanContext::GetVulkanDevice().GetQueue(Queue::Compute));
		    vkQueueWaitIdle(VulkanContext::GetVulkanDevice().GetQueue(Queue::Present));
        }

        FreeQueue();
        m_SwapChain.Destroy();
		m_TaskManager.Destroy();
        FreeQueue();
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::BeginFrame()
    {
        LU_PROFILE("VkRenderer::BeginFrame");
        if (m_Specification.WindowRef->IsMinimized())
            return;

        // Free objects
        FreeQueue();

        // Handle synchronization
        {
            auto& fences = m_TaskManager.GetFences();
            if (!fences.empty()) 
            {
                LU_PROFILE("VkRenderer::BeginFrame::WaitFences");
                auto device = VulkanContext::GetVulkanDevice().GetVkDevice();

                vkWaitForFences(device, static_cast<uint32_t>(fences.size()), fences.data(), VK_TRUE, std::numeric_limits<uint64_t>::max());
                vkResetFences(device, static_cast<uint32_t>(fences.size()), fences.data());
            }
            m_TaskManager.ResetFences();

            m_TaskManager.Add(m_SwapChain.GetCurrentImageAvailableSemaphore());
        }

        // Start frame
        m_SwapChain.AcquireNextImage();
    }

    void VulkanRenderer::EndFrame()
    {
        LU_PROFILE("VkRenderer::EndFrame");
        if (m_Specification.WindowRef->IsMinimized())
            return;
    }

    void VulkanRenderer::Present()
    {
        LU_PROFILE("VkRenderer::Present");
        if (m_Specification.WindowRef->IsMinimized())
            return;

        auto& semaphores = m_TaskManager.GetSemaphores();

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = static_cast<uint32_t>(semaphores.size());
        presentInfo.pWaitSemaphores = semaphores.data();
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_SwapChain.m_SwapChain;
        presentInfo.pImageIndices = &m_SwapChain.m_AcquiredImage;
        presentInfo.pResults = nullptr; // Optional

        VkResult result = VK_SUCCESS;
        {
            LU_PROFILE("VkRenderer::QueuePresent");
            result = vkQueuePresentKHR(VulkanContext::GetVulkanDevice().GetPresentQueue(), &presentInfo);
        }

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) 
        {
            Recreate(m_Specification.WindowRef->GetSize().x, m_Specification.WindowRef->GetSize().y, m_Specification.VSync);
        }
        else if (result != VK_SUCCESS) 
        {
            LU_LOG_ERROR("[VulkanRenderer] Failed to present swap chain image!");
        }

        m_TaskManager.ResetSemaphores();
        m_SwapChain.m_CurrentFrame = (m_SwapChain.m_CurrentFrame + 1) % static_cast<uint32_t>(m_Specification.Buffers);
    }

    void VulkanRenderer::BeginDynamic(CommandBuffer& cmdBuf, const DynamicRenderState& state)
    {
        LU_PROFILE("VkRenderer::BeginDynamic");
        VulkanCommandBuffer& vkCmdBuf = cmdBuf.GetInternalCommandBuffer();

        VkRenderingAttachmentInfo colourAttachment = {};
        colourAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colourAttachment.imageView = (state.ColourAttachment ? state.ColourAttachment->GetInternalImage().GetVkImageView() : VK_NULL_HANDLE);
        colourAttachment.imageLayout = (state.ColourAttachment ? ImageLayoutToVkImageLayout(state.ColourAttachment->GetSpecification().Layout) : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        colourAttachment.loadOp = LoadOperationToVkAttachmentLoadOp(state.ColourLoadOp);
        colourAttachment.storeOp = StoreOperationToVkAttachmentStoreOp(state.ColourStoreOp);
        colourAttachment.clearValue = { state.ColourClearValue.r, state.ColourClearValue.g, state.ColourClearValue.b, state.ColourClearValue.a };

        VkRenderingAttachmentInfo depthAttachment = {};
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = (state.DepthAttachment ? state.DepthAttachment->GetInternalImage().GetVkImageView() : VK_NULL_HANDLE);
        depthAttachment.imageLayout = (state.DepthAttachment ? ImageLayoutToVkImageLayout(state.DepthAttachment->GetSpecification().Layout) : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        depthAttachment.loadOp = LoadOperationToVkAttachmentLoadOp(state.DepthLoadOp);
        depthAttachment.storeOp = StoreOperationToVkAttachmentStoreOp(state.DepthStoreOp);
        depthAttachment.clearValue = { state.DepthClearValue };

        uint32_t width = 0, height = 0;
        if (state.ColourAttachment)
        {
            width = state.ColourAttachment->GetSpecification().Width;
            height = state.ColourAttachment->GetSpecification().Height;
        }
        else if (state.DepthAttachment)
        {
            width = state.DepthAttachment->GetSpecification().Width;
            height = state.DepthAttachment->GetSpecification().Height;
        }
        else
        {
            LU_ASSERT(false, "[VulkanRenderer] No Colour or Depth attachment passed in to BeginDynamic(..., state)");
        }

        VkRenderingInfo renderingInfo = {};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea.offset = { 0, 0 };
        renderingInfo.renderArea.extent = { width, height };
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = (state.ColourAttachment ? 1 : 0);
        renderingInfo.pColorAttachments = (state.ColourAttachment ? &colourAttachment : nullptr);
        renderingInfo.pDepthAttachment = (state.DepthAttachment ? &depthAttachment : nullptr);

        vkCmdBeginRendering(vkCmdBuf.GetVkCommandBuffer(m_SwapChain.GetCurrentFrame()), &renderingInfo);
    }

    void VulkanRenderer::EndDynamic(CommandBuffer& cmdBuf)
    {
        LU_PROFILE("VkRenderer::EndDynamic");
        VulkanCommandBuffer& vkCmdBuf = cmdBuf.GetInternalCommandBuffer();

        vkCmdEndRendering(vkCmdBuf.GetVkCommandBuffer(m_SwapChain.GetCurrentFrame()));
    }

    void VulkanRenderer::SetViewportAndScissor(CommandBuffer& cmdBuf, uint32_t width, uint32_t height)
    {
        LU_PROFILE("VkRenderer::SetViewportAndScissor");
        VulkanCommandBuffer& vkCmdBuf = cmdBuf.GetInternalCommandBuffer();

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)width;
        viewport.height = (float)height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(vkCmdBuf.m_CommandBuffers[m_SwapChain.GetCurrentFrame()], 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = { width, height };
        vkCmdSetScissor(vkCmdBuf.m_CommandBuffers[m_SwapChain.GetCurrentFrame()], 0, 1, &scissor);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Object methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::Begin(CommandBuffer& cmdBuf)
    {
        LU_PROFILE("VkRenderer::Begin(CommandBuffer)");
        VulkanCommandBuffer& vkCmdBuf = cmdBuf.GetInternalCommandBuffer();

        uint32_t currentFrame = m_SwapChain.GetCurrentFrame();
        VkCommandBuffer commandBuffer = vkCmdBuf.m_CommandBuffers[currentFrame];

        {
            LU_PROFILE("VkRenderer::Begin::ResetFences");
            vkResetFences(VulkanContext::GetVulkanDevice().GetVkDevice(), 1, &vkCmdBuf.m_InFlightFences[currentFrame]);
        }
        {
            LU_PROFILE("VkRenderer::Begin::ResetCmdBuf");
            vkResetCommandBuffer(commandBuffer, 0);
        }

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        {
            LU_PROFILE("VkRenderer::Begin::BeginCmdBuf");
            VK_VERIFY(vkBeginCommandBuffer(commandBuffer, &beginInfo));
        }
    }

    void VulkanRenderer::Begin(Renderpass& renderpass)
    {
        LU_PROFILE("VkRenderer::Begin(Renderpass)");
        CommandBuffer& cmdBuf = renderpass.GetCommandBuffer();
        VulkanRenderpass& vkRenderpass = renderpass.GetInternalRenderpass();
        VulkanCommandBuffer& vkCmdBuf = cmdBuf.GetInternalCommandBuffer();

        Begin(cmdBuf);

        auto size = renderpass.GetSize();
        VkExtent2D extent = { size.x, size.y };

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vkRenderpass.m_RenderPass;
        renderPassInfo.framebuffer = vkRenderpass.m_Framebuffers[m_SwapChain.GetAquiredImage()];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = extent;

        std::vector<VkClearValue> clearValues = {};
        if (!vkRenderpass.m_Specification.ColourAttachment.empty())
        {
            VkClearValue colourClear = { { { vkRenderpass.m_Specification.ColourClearColour.r, vkRenderpass.m_Specification.ColourClearColour.g, vkRenderpass.m_Specification.ColourClearColour.b, vkRenderpass.m_Specification.ColourClearColour.a } } };
            clearValues.push_back(colourClear);
        }
        if (vkRenderpass.m_Specification.DepthAttachment)
        {
            VkClearValue depthClear = { { { 1.0f, 0 } } };
            clearValues.push_back(depthClear);
        }

        renderPassInfo.clearValueCount = (uint32_t)clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        {
            LU_PROFILE("VkRenderer::Begin::BeginPass");
            vkCmdBeginRenderPass(vkCmdBuf.m_CommandBuffers[m_SwapChain.GetCurrentFrame()], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        }

        SetViewportAndScissor(cmdBuf, extent.width, extent.height);
    }

    void VulkanRenderer::End(CommandBuffer& cmdBuf)
    {
        LU_PROFILE("VkRenderer::End(CommandBuffer)");
        VulkanCommandBuffer& vkCmdBuf = cmdBuf.GetInternalCommandBuffer();

        {
            LU_PROFILE("VkRenderer::End::EndCmdBuf");
            VK_VERIFY(vkEndCommandBuffer(vkCmdBuf.m_CommandBuffers[m_SwapChain.GetCurrentFrame()]));
        }
    }

    void VulkanRenderer::End(Renderpass& renderpass)
    {
        LU_PROFILE("VkRenderer::End(Renderpass)");
        VulkanCommandBuffer& vkCmdBuf = renderpass.GetCommandBuffer().GetInternalCommandBuffer();
        
        {
            LU_PROFILE("VkRenderer::End::EndPass");
            vkCmdEndRenderPass(vkCmdBuf.m_CommandBuffers[m_SwapChain.GetCurrentFrame()]);
        }

        End(renderpass.GetCommandBuffer());
    }

    void VulkanRenderer::Submit(CommandBuffer& cmdBuf, ExecutionPolicy policy, Queue queue, PipelineStage waitStage, const std::vector<CommandBuffer*>& waitOn)
    {
        LU_PROFILE("VkRenderer::Submit(CommandBuffer)");
        VulkanCommandBuffer& vkCmdBuf = cmdBuf.GetInternalCommandBuffer();

        uint32_t currentFrame = m_SwapChain.GetCurrentFrame();
        VkCommandBuffer commandBuffer = vkCmdBuf.m_CommandBuffers[currentFrame];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        std::vector<VkSemaphore> semaphores = { };
        for (auto& cmd : waitOn)
        {
            VulkanCommandBuffer& vkCmd = cmd->GetInternalCommandBuffer();
            auto semaphore = vkCmd.m_RenderFinishedSemaphores[currentFrame];

            semaphores.push_back(semaphore);
            m_TaskManager.Remove(semaphore); // Removes it if it exists
        }

        if (!(policy & ExecutionPolicy::NoWaiting))
        {
            auto semaphore = m_TaskManager.GetNext();

            // Check if it's not nullptr
            if (semaphore)
            {
                #if !defined(LU_CONFIG_DIST) // Check if semaphore not already exists
				bool exists = false;
                for (const auto& sem : semaphores)
                {
					if (sem == semaphore) [[unlikely]]
					{
						LU_LOG_WARN("[VulkanRenderer] Semaphore already exists in the waitOn list!");
						exists = true;
						break;
					}
                }
                #endif

				if (!exists) [[unlikely]]
                    semaphores.push_back(semaphore);
            }
        }

        std::vector<VkPipelineStageFlags> waitStages(semaphores.size(), (VkPipelineStageFlagBits)waitStage);

        submitInfo.waitSemaphoreCount = static_cast<uint32_t>(semaphores.size());
        submitInfo.pWaitSemaphores = semaphores.data();
        submitInfo.pWaitDstStageMask = waitStages.data();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &vkCmdBuf.m_RenderFinishedSemaphores[currentFrame];

        // Submission
        {
            LU_PROFILE("VkRenderer::Submit::QueueSubmit");
            VK_VERIFY(vkQueueSubmit(VulkanContext::GetVulkanDevice().GetQueue(queue), 1, &submitInfo, vkCmdBuf.m_InFlightFences[currentFrame]));
        }
        m_TaskManager.Add(vkCmdBuf, policy);
    }

    void VulkanRenderer::Submit(Renderpass& renderpass, ExecutionPolicy policy, Queue queue, PipelineStage waitStage, const std::vector<CommandBuffer*>& waitOn)
    {
        LU_PROFILE("VkRenderer::Submit(Renderpass)");
        Submit(renderpass.GetCommandBuffer(), policy, queue, waitStage, waitOn);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Internal methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::Free(const FreeFn& fn)
    {
        std::scoped_lock<std::mutex> lock(m_FreeMutex);
        m_FreeQueue.push(fn);
    }

    void VulkanRenderer::FreeQueue()
    {
        LU_PROFILE("VkRenderer::FreeQueue");
        std::scoped_lock<std::mutex> lock(m_FreeMutex);
        while (!m_FreeQueue.empty())
        {
            m_FreeQueue.front()();
            m_FreeQueue.pop();
        }
    }

    void VulkanRenderer::Recreate(uint32_t width, uint32_t height, bool vsync)
    {
        m_SwapChain.Resize(width, height, vsync, static_cast<uint8_t>(m_Specification.Buffers));
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Getters
    ////////////////////////////////////////////////////////////////////////////////////
    std::vector<Image*> VulkanRenderer::GetSwapChainImages()
    {
        std::vector<Image*> images(m_SwapChain.m_Images.size());
        
        for (size_t i = 0; i < images.size(); i++)
            images[i] = Vk::Cast<Image>(&m_SwapChain.m_Images[i]);

        return images;
    }

    Image* VulkanRenderer::GetDepthImage()
    {
        return Vk::Cast<Image>(&m_SwapChain.m_DepthStencil);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Static methods
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanRenderer& VulkanRenderer::GetRenderer(RendererID id)
    {
		if constexpr (Info::g_RenderingAPI == Info::RenderingAPI::Vulkan)
            return Renderer::GetRenderer(id).GetInternalRenderer();
    }

}