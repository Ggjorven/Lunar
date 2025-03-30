#pragma once

#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/Renderer/RendererSpec.hpp"
#include "Lunar/Internal/Renderer/ImageSpec.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanImage.hpp"

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Selection
    ////////////////////////////////////////////////////////////////////////////////////
    template<Info::RenderingAPI API>
    struct ImageSelect;

    template<> struct ImageSelect<Info::RenderingAPI::Vulkan> { using Type = VulkanImage; };

    using ImageType = typename ImageSelect<Info::g_RenderingAPI>::Type;

    ////////////////////////////////////////////////////////////////////////////////////
    // Image
    ////////////////////////////////////////////////////////////////////////////////////
    class Image
    {
    public:
        // Constructor & Destructor
		inline Image(const RendererID renderer, const ImageSpecification& specs, const SamplerSpecification& samplerSpecs) { m_Image.Init(renderer, specs, samplerSpecs); }
		inline ~Image() { m_Image.Destroy(); }

        // Methods
		inline void SetData(void* data, size_t size) { m_Image.SetData(data, size); }

		inline void Resize(uint32_t width, uint32_t height) { m_Image.Resize(width, height); }

		inline void Transition(ImageLayout initial, ImageLayout final) { m_Image.Transition(initial, final); }

        // Getters
        inline RendererID GetRendererID() const { return m_Image.GetRendererID(); }
        inline const ImageSpecification& GetSpecification() const { return m_Image.GetSpecification(); }
        inline const SamplerSpecification& GetSamplerSpecification() const { return m_Image.GetSamplerSpecification(); }

		inline uint32_t GetWidth() const { return m_Image.GetWidth(); }
		inline uint32_t GetHeight() const { return m_Image.GetHeight(); }

        // Internal
        // Note: This is an internal function, do not call.
        inline ImageType& GetInternalImage() { return m_Image; }

    private:
        ImageType m_Image = {};
    };

}