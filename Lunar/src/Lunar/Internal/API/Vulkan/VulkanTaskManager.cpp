#include "lupch.h"
#include "VulkanTaskManager.hpp"

#include "Lunar/Internal/IO/Print.hpp"
#include "Lunar/Internal/Utils/Profiler.hpp"

#include "Lunar/Internal/Renderer/Renderer.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanTaskManager::Init(RendererID rendererID, uint32_t frameCount) 
    {
		m_RendererID = rendererID;

        m_Semaphores.resize(frameCount);
        m_Fences.resize(frameCount);
    }

	void VulkanTaskManager::Destroy()
	{
	}

    ////////////////////////////////////////////////////////////////////////////////////
    // Add methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanTaskManager::Add(VulkanCommandBuffer& cmdBuf, ExecutionPolicy policy)
    {
        LU_PROFILE("VkTaskManager::Add(Cmd, Policy)");
        std::scoped_lock<std::mutex> lock(m_ThreadSafety);

        uint32_t frame = VulkanRenderer::GetRenderer(m_RendererID).GetVulkanSwapChain().GetCurrentFrame();
        m_Fences[frame].emplace_back(cmdBuf.GetVkInFlightFence(frame));

        if (policy & ExecutionPolicy::InOrder)
            m_Semaphores[frame].first.push_back(cmdBuf.GetVkRenderFinishedSemaphore(frame));
        else if (policy & ExecutionPolicy::Parallel)
            m_Semaphores[frame].second.push_back(cmdBuf.GetVkRenderFinishedSemaphore(frame));
    }

    void VulkanTaskManager::Add(VkSemaphore semaphore)
    {
        LU_PROFILE("VkTaskManager::Add(Semaphore)");
        std::scoped_lock<std::mutex> lock(m_ThreadSafety);

        uint32_t frame = VulkanRenderer::GetRenderer(m_RendererID).GetVulkanSwapChain().GetCurrentFrame();
        m_Semaphores[frame].first.push_back(semaphore);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Remove & Reset methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanTaskManager::Remove(VkFence fence)
    {
        Remove(fence, VulkanRenderer::GetRenderer(m_RendererID).GetVulkanSwapChain().GetCurrentFrame());
    }

    void VulkanTaskManager::Remove(VkFence fence, uint32_t frame)
    {
        LU_PROFILE("VkTaskManager::Remove(Fence)");
        std::scoped_lock<std::mutex> lock(m_ThreadSafety);

        auto it = std::find(m_Fences[frame].begin(), m_Fences[frame].end(), fence);
        if (it != m_Fences[frame].end()) 
        {
            m_Fences[frame].erase(it);
            return;
        }
    }

    void VulkanTaskManager::Remove(VkSemaphore semaphore)
    {
        Remove(semaphore, VulkanRenderer::GetRenderer(m_RendererID).GetVulkanSwapChain().GetCurrentFrame());
    }

    void VulkanTaskManager::Remove(VkSemaphore semaphore, uint32_t frame)
    {
        LU_PROFILE("VkTaskManager::Remove(Semaphore)");
        std::scoped_lock<std::mutex> lock(m_ThreadSafety);

        // Check the InOrder list
        auto it = std::find(m_Semaphores[frame].first.begin(), m_Semaphores[frame].first.end(), semaphore);
        if (it != m_Semaphores[frame].first.end()) 
        {
            m_Semaphores[frame].first.erase(it);
            return;
        }

        // Check the Frame End list
        auto it2 = std::find(m_Semaphores[frame].second.begin(), m_Semaphores[frame].second.end(), semaphore);
        if (it2 != m_Semaphores[frame].second.end()) 
        {
            m_Semaphores[frame].second.erase(it2);
            return;
        }
    }

    void VulkanTaskManager::RemoveFromAll(VkFence fence)
    {
        LU_PROFILE("VkTaskManager::RemoveFromAll(Semaphore)");
        std::scoped_lock<std::mutex> lock(m_ThreadSafety);

        for (uint32_t frame = 0; frame < (uint32_t)VulkanRenderer::GetRenderer(m_RendererID).GetSpecification().Buffers; frame++)
            Remove(fence, frame);
    }

    void VulkanTaskManager::RemoveFromAll(VkSemaphore semaphore)
    {
        LU_PROFILE("VkTaskManager::RemoveFromAll(Semaphore)");
        std::scoped_lock<std::mutex> lock(m_ThreadSafety);

        for (uint32_t frame = 0; frame < (uint32_t)VulkanRenderer::GetRenderer(m_RendererID).GetSpecification().Buffers; frame++)
            Remove(semaphore, frame);
    }

    void VulkanTaskManager::ResetFences()
    {
        LU_PROFILE("VkTaskManager::ResetFences");
        uint32_t frame = VulkanRenderer::GetRenderer(m_RendererID).GetVulkanSwapChain().GetCurrentFrame();
        m_Fences[frame].clear();
    }

    void VulkanTaskManager::ResetSemaphores()
    {
        LU_PROFILE("VkTaskManager::ResetSemaphores");

        uint32_t frame = VulkanRenderer::GetRenderer(m_RendererID).GetVulkanSwapChain().GetCurrentFrame();

        m_Semaphores[frame].first.clear();
        m_Semaphores[frame].second.clear();
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Getters
    ////////////////////////////////////////////////////////////////////////////////////
    VkSemaphore VulkanTaskManager::GetNext()
    {
        LU_PROFILE("VkTaskManager::GetNext");
        std::scoped_lock<std::mutex> lock(m_ThreadSafety);

        uint32_t frame = VulkanRenderer::GetRenderer(m_RendererID).GetVulkanSwapChain().GetCurrentFrame();
        if (m_Semaphores[frame].first.empty())
            return VK_NULL_HANDLE;

        VkSemaphore semaphore = m_Semaphores[frame].first[0];
        m_Semaphores[frame].first.erase(m_Semaphores[frame].first.begin() + 0);

        return semaphore;
    }

    std::vector<VkFence>& VulkanTaskManager::GetFences()
    {
        return m_Fences[VulkanRenderer::GetRenderer(m_RendererID).GetVulkanSwapChain().GetCurrentFrame()];
    }

    std::vector<VkSemaphore>& VulkanTaskManager::GetSemaphores()
    {
        static std::vector<VkSemaphore> semaphores = {};
        semaphores.clear();

        uint32_t frame = VulkanRenderer::GetRenderer(m_RendererID).GetVulkanSwapChain().GetCurrentFrame();

        semaphores.reserve(m_Semaphores[frame].first.size() + m_Semaphores[frame].second.size()); // Reserve space to avoid multiple allocations
        semaphores.insert(semaphores.end(), m_Semaphores[frame].first.begin(), m_Semaphores[frame].first.end());
        semaphores.insert(semaphores.end(), m_Semaphores[frame].second.begin(), m_Semaphores[frame].second.end());

        return semaphores;
    }

}