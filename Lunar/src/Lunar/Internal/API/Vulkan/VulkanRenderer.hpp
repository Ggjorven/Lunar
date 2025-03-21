#pragma once

#include <cstdint>
#include <queue>
#include <mutex>

#include "Lunar/Internal/Renderer/RendererSpec.hpp"

#include "Lunar/Internal/API/Vulkan/Vulkan.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanSwapChain.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanTaskManager.hpp"

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanRenderer
    ////////////////////////////////////////////////////////////////////////////////////
	class VulkanRenderer
	{
    public:
        // Constructor & Destructor
		VulkanRenderer() = default;
        ~VulkanRenderer() = default;

		// Init & Destroy
        void Init(const RendererSpecification& specs, RendererID id);
        void Destroy();

		// Methods
        void BeginFrame();
        void EndFrame();
        void Present();

        void Free(const FreeFn& fn);
        void FreeQueue();

        // TODO: Submit

        void Recreate(uint32_t width, uint32_t height, bool vsync);

		// Getters
        inline RendererID GetID() const { return m_ID; }
        inline const RendererSpecification& GetSpecification() const { return m_Specification; }
        inline VulkanTaskManager& GetTaskManager() { return m_TaskManager; }

        inline VulkanSwapChain& GetVulkanSwapChain() { return m_SwapChain; }

        // Static methods
        static VulkanRenderer& GetRenderer(RendererID id);

	private:
        RendererID m_ID = 0;
        RendererSpecification m_Specification;

        VulkanSwapChain m_SwapChain = {};

        VulkanTaskManager m_TaskManager = {};

        std::mutex m_FreeMutex = {};
        std::queue<FreeFn> m_FreeQueue = {};
	};

}