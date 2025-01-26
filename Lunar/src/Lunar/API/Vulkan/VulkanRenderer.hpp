#pragma once

#include <cstdint>
#include <queue>
#include <mutex>

#include "Lunar/Renderer/Renderer.hpp"

#include "Lunar/API/Vulkan/Vulkan.hpp"
#include "Lunar/API/Vulkan/VulkanSwapChain.hpp"
#include "Lunar/API/Vulkan/VulkanTaskManager.hpp"

namespace Lunar
{

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanRenderer
    ////////////////////////////////////////////////////////////////////////////////////
	class VulkanRenderer : public Renderer
	{
    public:
        // Constructor & Destructor
		VulkanRenderer(const RendererSpecification& specs);
        ~VulkanRenderer();

		// Methods
        void BeginFrame() override;
        void EndFrame() override;
        void Present() override;

        void Free(const FreeFn& fn) override;
        void FreeQueue() override;

        // TODO: Submit

        void Recreate(uint32_t width, uint32_t height, bool vsync) override;

		// Getters
        inline RendererID GetID() const { return m_ID; }
        inline const RendererSpecification& GetSpecification() const override { return m_Specification; }
        inline VulkanTaskManager& GetTaskManager() { return m_TaskManager; }

        inline Arc<VulkanSwapChain> GetVulkanSwapChain() { return m_SwapChain; }

    public:
        // Static methods
        static VulkanRenderer& GetRenderer(RendererID id);

	private:
        RendererID m_ID;
        RendererSpecification m_Specification;

        Arc<VulkanSwapChain> m_SwapChain = nullptr;

        VulkanTaskManager m_TaskManager;

        std::mutex m_FreeMutex = {};
        std::queue<FreeFn> m_FreeQueue = {};
	};

}