#pragma once

#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/Renderer/RendererSpec.hpp"
#include "Lunar/Internal/Renderer/ImageSpec.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanImage.hpp"

#include <filesystem>

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
        inline Image() = default;
		inline Image(const RendererID renderer, const ImageSpecification& specs, const SamplerSpecification& samplerSpecs) { Init(renderer, specs, samplerSpecs); }
		inline Image(const RendererID renderer, const ImageSpecification& specs, const SamplerSpecification& samplerSpecs, const std::filesystem::path& imagePath) { Init(renderer, specs, samplerSpecs, imagePath); }
        inline ~Image() = default;

        // Init & Destroy
		inline void Init(const RendererID renderer, const ImageSpecification& specs, const SamplerSpecification& samplerSpecs) { m_Image.Init(renderer, specs, samplerSpecs); }
		inline void Init(const RendererID renderer, const ImageSpecification& specs, const SamplerSpecification& samplerSpecs, const std::filesystem::path& imagePath) { m_Image.Init(renderer, specs, samplerSpecs, imagePath); }
		inline void Destroy(const RendererID renderer) { m_Image.Destroy(renderer); }

        // Methods
		inline void SetData(const RendererID renderer, void* data, size_t size) { m_Image.SetData(renderer, data, size); }

		inline void Resize(const RendererID renderer, uint32_t width, uint32_t height) { m_Image.Resize(renderer, width, height); }

		inline void Transition(const RendererID renderer, ImageLayout initial, ImageLayout final) { m_Image.Transition(renderer, initial, final); }

        // Getters
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