#include "lupch.h"
#include "VulkanRenderer.hpp"

#include "Lunar/Internal/IO/Print.hpp"
#include "Lunar/Internal/Utils/Profiler.hpp"
#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/Core/Window.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"

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
        FreeQueue();
        m_SwapChain.Destroy();
        FreeQueue();
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::BeginFrame()
    {
        LU_PROFILE("VulkanRenderer::BeginFrame");
        if (m_Specification.WindowRef->IsMinimized())
            return;

        // Free objects
        FreeQueue();

        // Handle synchronization
        {
            auto& fences = m_TaskManager.GetFences();
            if (!fences.empty()) 
            {
                auto device = VulkanContext::GetVulkanDevice().GetVkDevice();

                vkWaitForFences(device, (uint32_t)fences.size(), fences.data(), VK_TRUE, std::numeric_limits<uint64_t>::max());
                vkResetFences(device, (uint32_t)fences.size(), fences.data());
            }
            m_TaskManager.ResetFences();

            m_TaskManager.Add(m_SwapChain.GetCurrentImageAvailableSemaphore());
        }

        // Start frame
        m_SwapChain.AcquireNextImage();
    }

    void VulkanRenderer::EndFrame()
    {
        LU_PROFILE("VulkanRenderer::EndFrame");
        if (m_Specification.WindowRef->IsMinimized())
            return;
    }

    void VulkanRenderer::Present()
    {
        LU_PROFILE("VulkanRenderer::Present");
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
            // Note: Without this line there is a memory leak on windows when validation layers are enabled.
            #if defined(LU_PLATFORM_WINDOWS)
            if constexpr (g_VkValidation) 
            {
                LU_PROFILE("VkRenderer::WaitIdle");
                vkQueueWaitIdle(VulkanContext::GetVulkanDevice().GetGraphicsQueue());
            }
            #endif

            {
                LU_PROFILE("VkRenderer::QueuePresent");
                result = vkQueuePresentKHR(VulkanContext::GetVulkanDevice().GetPresentQueue(), &presentInfo);
            }
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
        m_SwapChain.m_CurrentFrame = (m_SwapChain.m_CurrentFrame + 1) % (uint32_t)m_Specification.Buffers;
    }

    void VulkanRenderer::Free(const FreeFn& fn)
    {
        std::scoped_lock<std::mutex> lock(m_FreeMutex);
        m_FreeQueue.push(fn);
    }

    void VulkanRenderer::FreeQueue()
    {
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
    // Static methods
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanRenderer& VulkanRenderer::GetRenderer(RendererID id)
    {
		if constexpr (Info::g_RenderingAPI == Info::RenderingAPI::Vulkan)
            return Renderer::GetRenderer(id).GetInternalRenderer();
    }

}