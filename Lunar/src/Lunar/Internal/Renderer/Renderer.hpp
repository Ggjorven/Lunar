#pragma once

#include "Lunar/Internal/Renderer/RendererSpec.hpp"

#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/Renderer/Buffers.hpp"
#include "Lunar/Internal/Renderer/Pipeline.hpp"
#include "Lunar/Internal/Renderer/Renderpass.hpp"
#include "Lunar/Internal/Renderer/CommandBuffer.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanRenderer.hpp"

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Selection
    ////////////////////////////////////////////////////////////////////////////////////
    template<Info::RenderingAPI API>
    struct RendererSelect;

    template<> struct RendererSelect<Info::RenderingAPI::Vulkan> { using Type = VulkanRenderer; };

	using RendererType = typename RendererSelect<Info::g_RenderingAPI>::Type;

    ////////////////////////////////////////////////////////////////////////////////////
    // Renderer
    ////////////////////////////////////////////////////////////////////////////////////
    class Renderer
    {
    public:
        // Constructor & Destructor
        Renderer();
        ~Renderer();

		// Init & Destroy
		inline void Init(const RendererSpecification& specs) { m_Renderer.Init(specs, m_ID); }
        inline void Destroy() { m_Renderer.Destroy(); }
      
        // Methods
        inline void BeginFrame() { m_Renderer.BeginFrame(); }
        inline void EndFrame() { m_Renderer.EndFrame(); }
        inline void Present() { m_Renderer.Present(); }

        // Note: These are only when you actually want to use dynamic rendering, 'static' rendering using renderpasses can be done just with Begin() etc.
        // Note 2: You still have to call Begin, End & Submit for the CommandBuffer yourself.
        inline void BeginDynamic(CommandBuffer& cmdBuf, const DynamicRenderState& state) { m_Renderer.BeginDynamic(cmdBuf, state); }
        inline void EndDynamic(CommandBuffer& cmdBuf) { m_Renderer.EndDynamic(cmdBuf); }

        // Object methods
		inline void Begin(CommandBuffer& cmdBuf) { m_Renderer.Begin(cmdBuf); }
		inline void Begin(Renderpass& renderpass) { m_Renderer.Begin(renderpass); }
		inline void End(CommandBuffer& cmdBuf) { m_Renderer.End(cmdBuf); }
		inline void End(Renderpass& renderpass) { m_Renderer.End(renderpass); }
        inline void Submit(CommandBuffer& cmdBuf, ExecutionPolicy policy, Queue queue = Queue::Graphics, PipelineStage waitStage = PipelineStage::ColourAttachmentOutput, const std::vector<CommandBuffer*>& waitOn = {}) { m_Renderer.Submit(cmdBuf, policy, queue, waitStage, waitOn); }
        inline void Submit(Renderpass& renderpass, ExecutionPolicy policy, Queue queue = Queue::Graphics, PipelineStage waitStage = PipelineStage::ColourAttachmentOutput, const std::vector<CommandBuffer*>& waitOn = {}) { m_Renderer.Submit(renderpass, policy, queue, waitStage, waitOn); }

		inline void Draw(CommandBuffer& cmdBuf, uint32_t vertexCount = 3, uint32_t instanceCount = 1) { m_Renderer.Draw(cmdBuf, vertexCount, instanceCount); }
		inline void DrawIndexed(CommandBuffer& cmdBuf, uint32_t indexCount, uint32_t instanceCount = 1) { m_Renderer.DrawIndexed(cmdBuf, indexCount, instanceCount); }
		inline void DrawIndexed(CommandBuffer& cmdBuf, IndexBuffer& indexBuffer, uint32_t instanceCount = 1) { m_Renderer.DrawIndexed(cmdBuf, indexBuffer, instanceCount); }

        // Internal
        inline void Free(const FreeFn& fn) { m_Renderer.Free(fn); }
        inline void FreeQueue() { m_Renderer.FreeQueue(); }
        
        inline void Recreate(uint32_t width, uint32_t height, bool vsync) { m_Renderer.Recreate(width, height, vsync); }

        // Getters
		inline RendererID GetID() const { return m_ID; }
		inline const RendererSpecification& GetSpecification() const { return m_Renderer.GetSpecification(); }

        inline std::vector<Image*> GetSwapChainImages() { return m_Renderer.GetSwapChainImages(); }
        inline Image* GetDepthImage() { return m_Renderer.GetDepthImage(); }

        // Internal
        // Note: This is an internal function, do not call.
        inline RendererType& GetInternalRenderer() { return m_Renderer; }

		// Static methods
        static Renderer& GetRenderer(RendererID id);

    private:
		const RendererID m_ID;
        RendererType m_Renderer = {};
    };

}