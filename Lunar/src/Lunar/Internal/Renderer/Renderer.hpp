#pragma once

#include "Lunar/Internal/Renderer/RendererSpec.hpp"

#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanRenderer.hpp"

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Selection
    ////////////////////////////////////////////////////////////////////////////////////
    template<Info::RenderingAPI API>
    struct RendererSelect;

    template<> struct RendererSelect<Info::RenderingAPI::Vulkan> { using Type = typename VulkanRenderer; };

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

        inline void Free(const FreeFn& fn) { m_Renderer.Free(fn); }
        inline void FreeQueue() { m_Renderer.FreeQueue(); }
        
        // TODO: Submit

        inline void Recreate(uint32_t width, uint32_t height, bool vsync) { m_Renderer.Recreate(width, height, vsync); }

        // Getters
		inline RendererID GetID() const { return m_ID; }
		inline const RendererSpecification& GetSpecification() const { return m_Renderer.GetSpecification(); }

        // Note: This is an internal function, do not call.
        inline RendererType& GetInternalRenderer() { return m_Renderer; }

		// Static methods
        static Renderer& GetRenderer(RendererID id);

    private:
		const RendererID m_ID;
        RendererType m_Renderer = {};
    };

}