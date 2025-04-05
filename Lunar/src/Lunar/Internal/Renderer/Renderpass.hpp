#pragma once

#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/Renderer/RendererSpec.hpp"
#include "Lunar/Internal/Renderer/RenderpassSpec.hpp"
#include "Lunar/Internal/Renderer/CommandBuffer.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanRenderpass.hpp"

#include "Lunar/Maths/Structs.hpp"

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Selection
    ////////////////////////////////////////////////////////////////////////////////////
    template<Info::RenderingAPI API>
    struct RenderpassSelect;

    template<> struct RenderpassSelect<Info::RenderingAPI::Vulkan> { using Type = VulkanRenderpass; };

    using RenderpassType = typename RenderpassSelect<Info::g_RenderingAPI>::Type;

    ////////////////////////////////////////////////////////////////////////////////////
    // Renderpass
    ////////////////////////////////////////////////////////////////////////////////////
    class Renderpass
    {
    public:
        // Constructor & Destructor
        Renderpass(const RendererID renderer, const RenderpassSpecification& specs, CommandBuffer* cmdBuf = nullptr) { m_Renderpass.Init(renderer, specs, cmdBuf); }
		~Renderpass() { m_Renderpass.Destroy(); }

        // The Begin, End & Submit function are in the Renderer

        // Methods
        inline void Resize(uint32_t width, uint32_t height) { return m_Renderpass.Resize(width, height); }

        // Getters
        inline Vec2<uint32_t> GetSize() const { return m_Renderpass.GetSize(); }

        inline const RenderpassSpecification& GetSpecification() const { return m_Renderpass.GetSpecification(); }
		inline CommandBuffer& GetCommandBuffer() { return m_Renderpass.GetCommandBuffer(); }

        // Internal
        inline RenderpassType& GetInternalRenderpass() { return m_Renderpass; }

    private:
		RenderpassType m_Renderpass = {};
    };

}