#pragma once

#include "Lunar/Renderer/Renderer.hpp"

#include "Lunar/API/Vulkan/Vulkan.hpp"
#include "Lunar/API/Vulkan/VulkanCommandBuffer.hpp"

#include <cstdint>
#include <vector>

namespace Lunar
{

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanTaskManager
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanTaskManager
    {
    public:
        // Constructor & Destructor
        VulkanTaskManager(RendererID rendererID, uint32_t frameCount);
        ~VulkanTaskManager() = default;

        // Add methods
        void Add(Arc<VulkanCommandBuffer> cmdBuf, ExecutionPolicy policy);
        void Add(VkSemaphore semaphore); // Internal function for swapchain image available semaphore

        // Remove & Reset methods
        void Remove(VkFence fence); // It removes the fence from current frame if it exists
        void Remove(VkFence fence, uint32_t frame); // It removes the fence from the frame vector

        void Remove(VkSemaphore semaphore); // It removes the semaphore from current frame if it exists (it checks both vectors)
        void Remove(VkSemaphore semaphore, uint32_t frame); // It removes the semaphore from the frame vector
        
        void ResetFences(); // Resets current frame semaphores
        void ResetSemaphores(); // Resets current frame semaphores
        
        void RemoveFromAll(VkFence fence); // It removes the fence from all vectors if it exists
        void RemoveFromAll(VkSemaphore semaphore); // It removes the semaphore from all vectors if it exists

        // Getters
        VkSemaphore GetNext(); // Returns the next semaphore in the queue

        std::vector<VkFence>& GetFences();
        std::vector<VkSemaphore>& GetSemaphores();

    private:
        const RendererID m_RendererID;
        std::mutex m_ThreadSafety = {};

        // The first vector index is the frame, the pair is first = semaphores to wait on by other cmdBufs, second = semaphores to only wait on by frame
        std::vector<std::pair<std::vector<VkSemaphore>, std::vector<VkSemaphore>>> m_Semaphores = { };
        std::vector<std::vector<VkFence>> m_Fences = { }; // Waited on by Renderer::EndFrame
    };

}