#pragma once

#include <cstdint>
#include <queue>
#include <mutex>

#include "Lunar/Internal/Renderer/RendererSpec.hpp"
#include "Lunar/Internal/Renderer/Buffers.hpp"
#include "Lunar/Internal/Renderer/Pipeline.hpp"
#include "Lunar/Internal/Renderer/Renderpass.hpp"
#include "Lunar/Internal/Renderer/CommandBuffer.hpp"

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

        void BeginDynamic(CommandBuffer& cmdBuf, const DynamicRenderState& state);
        void EndDynamic(CommandBuffer& cmdBuf);

        void SetViewportAndScissor(CommandBuffer& cmdBuf, uint32_t width, uint32_t height);

        // Object methods
        void Begin(CommandBuffer& cmdBuf);
        void Begin(Renderpass& renderpass);
        void End(CommandBuffer& cmdBuf);
        void End(Renderpass& renderpass);
        void Submit(CommandBuffer& cmdBuf, ExecutionPolicy policy, Queue queue, PipelineStage waitStage, const std::vector<CommandBuffer*>& waitOn);
        void Submit(Renderpass& renderpass, ExecutionPolicy policy, Queue queue, PipelineStage waitStage, const std::vector<CommandBuffer*>& waitOn);

        void Draw(CommandBuffer& cmdBuf, uint32_t vertexCount, uint32_t instanceCount);
        void DrawIndexed(CommandBuffer& cmdBuf, uint32_t indexCount, uint32_t instanceCount);
        void DrawIndexed(CommandBuffer& cmdBuf, IndexBuffer& indexBuffer, uint32_t instanceCount);

        // Internal
        void Free(const FreeFn& fn);
        void FreeQueue();

        void Recreate(uint32_t width, uint32_t height, bool vsync);

		// Getters
        inline RendererID GetID() const { return m_ID; }
        inline const RendererSpecification& GetSpecification() const { return m_Specification; }

        ImageFormat GetColourFormat() const;
        ImageFormat GetDepthFormat() const;
        std::vector<Image*> GetSwapChainImages();

        // Internal getters
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