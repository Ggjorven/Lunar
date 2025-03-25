#pragma once

#include "Lunar/Internal/Renderer/RendererSpec.hpp"

#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/Renderer/CommandBuffer.hpp"
#include "Lunar/Internal/Renderer/PipelineSpec.hpp"

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

        // Object methods
		inline void Begin(CommandBuffer& cmdBuf) { m_Renderer.Begin(cmdBuf); }
		inline void End(CommandBuffer& cmdBuf) { m_Renderer.End(cmdBuf); }
        inline void Submit(CommandBuffer& cmdBuf, ExecutionPolicy policy, Queue queue = Queue::Graphics, PipelineStage waitStage = PipelineStage::ColourAttachmentOutput, const std::vector<CommandBuffer*>& waitOn = {}) { m_Renderer.Submit(cmdBuf, policy, queue, waitStage, waitOn); }

        // Internal
        inline void Free(const FreeFn& fn) { m_Renderer.Free(fn); }
        inline void FreeQueue() { m_Renderer.FreeQueue(); }
        
        inline void Recreate(uint32_t width, uint32_t height, bool vsync) { m_Renderer.Recreate(width, height, vsync); }

        // Getters
		inline RendererID GetID() const { return m_ID; }
		inline const RendererSpecification& GetSpecification() const { return m_Renderer.GetSpecification(); }

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