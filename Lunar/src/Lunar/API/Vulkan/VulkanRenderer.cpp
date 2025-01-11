#include "lupch.h"
#include "VulkanRenderer.hpp"

#include "Lunar/IO/Print.hpp"

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
        : m_ID(s_CurrentID++), m_Specification(specs)
    {
        s_Renderers[m_ID] = this;

        m_SwapChain = VulkanSwapChain::Create(specs.WindowRef);
    }

    VulkanRenderer::~VulkanRenderer()
    {
        m_SwapChain.Reset();

        s_Renderers[m_ID] = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanRenderer::Free(const FreeFn& fn)
    {
        // TODO: ...
        fn();
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