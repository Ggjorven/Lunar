#include "lupch.h"
#include "VulkanRenderer.hpp"

#include "Lunar/IO/Print.hpp"

#include "Lunar/Utils/Profiler.hpp"

#include "Lunar/API/Vulkan/VulkanContext.hpp"

#include <numeric>

namespace Lunar
{

    namespace
    {
        static RendererID s_CurrentID = 0;
        static std::array<VulkanRenderer*, std::numeric_limits<RendererID>::max()> s_Renderers = { }; 
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanRenderer::VulkanRenderer(const RendererSpecification& specs) 
        : m_ID(s_CurrentID++), m_Specification(specs), m_TaskManager(m_ID, (uint32_t)specs.Buffers)
    {
        s_Renderers[m_ID] = this;

        m_SwapChain = VulkanSwapChain::Create(specs.WindowRef);
    }

    VulkanRenderer::~VulkanRenderer()
    {
        FreeQueue();
        m_SwapChain.Reset();
        FreeQueue();

        s_Renderers[m_ID] = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::BeginFrame()
    {
        LU_PROFILE("VulkanRenderer::BeginFrame");
        if (m_Specification.WindowRef.IsMinimized())
            return;

        // Free objects
        FreeQueue();

        // Handle synchronization
        {
            auto& fences = m_TaskManager.GetFences();
            if (!fences.empty()) 
            {
                auto device = VulkanContext::GetVulkanDevice()->GetVkDevice();

                vkWaitForFences(device, (uint32_t)fences.size(), fences.data(), VK_TRUE, std::numeric_limits<uint64_t>::max());
                vkResetFences(device, (uint32_t)fences.size(), fences.data());
            }
            m_TaskManager.ResetFences();

            m_TaskManager.Add(m_SwapChain->GetCurrentImageAvailableSemaphore());
        }

        // Start frame
        {
            m_SwapChain->AcquireNextImage();
        }
    }

    void VulkanRenderer::EndFrame()
    {
        LU_PROFILE("VulkanRenderer::EndFrame");
        if (m_Specification.WindowRef.IsMinimized())
            return;
    }

    void VulkanRenderer::Present()
    {
        LU_PROFILE("VulkanRenderer::Present");
        if (m_Specification.WindowRef.IsMinimized())
            return;

        auto& semaphores = m_TaskManager.GetSemaphores();

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = (uint32_t)semaphores.size();
        presentInfo.pWaitSemaphores = semaphores.data();
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_SwapChain->m_SwapChain;
        presentInfo.pImageIndices = &m_SwapChain->m_AcquiredImage;
        presentInfo.pResults = nullptr; // Optional

        VkResult result = VK_SUCCESS;
        {
            // Note: Without this line there is a memory leak on windows when validation layers are enabled.
            #if defined(LU_PLATFORM_WINDOWS)
            if constexpr (g_VkValidation) 
            {
                LU_PROFILE("VkRenderer::WaitIdle");
                vkQueueWaitIdle(VulkanContext::GetVulkanDevice()->GetGraphicsQueue());
            }
            #endif

            {
                LU_PROFILE("VkRenderer::QueuePresent");
                result = vkQueuePresentKHR(VulkanContext::GetVulkanDevice()->GetPresentQueue(), &presentInfo);
            }
        }

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) 
        {
            Recreate(m_Specification.WindowRef.GetSize().x, m_Specification.WindowRef.GetSize().y, m_Specification.VSync);
        }
        else if (result != VK_SUCCESS) 
        {
            LU_LOG_ERROR("Failed to present swap chain image!");
        }

        m_TaskManager.ResetSemaphores();
        m_SwapChain->m_CurrentFrame = (m_SwapChain->m_CurrentFrame + 1) % (uint32_t)m_Specification.Buffers;
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
        m_SwapChain->Init(width, height, vsync, (uint8_t)m_Specification.Buffers);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Static methods
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanRenderer& VulkanRenderer::GetRenderer(RendererID id)
    {
        LU_ASSERT((static_cast<uint32_t>(s_CurrentID) > static_cast<uint32_t>(id)), "[VulkanRenderer] Tried to access a renderer by id which has not been handed out yet.");
        return *s_Renderers[id];
    }

}