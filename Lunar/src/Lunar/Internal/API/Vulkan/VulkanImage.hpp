#pragma once

#include "Lunar/Internal/API/Vulkan/Vulkan.hpp"

#include "Lunar/Internal/Renderer/RendererSpec.hpp"
#include "Lunar/Internal/Renderer/ImageSpec.hpp"

namespace Lunar::Internal
{

    class VulkanSwapChain;
    class VulkanDescriptorSet;

    ////////////////////////////////////////////////////////////////////////////////////
    // Convert functions
    ////////////////////////////////////////////////////////////////////////////////////
	ImageUsage VkImageUsageToImageUsage(VkImageUsageFlags usage);
	VkImageUsageFlags ImageUsageToVkImageUsage(ImageUsage usage);
	ImageLayout VkImageLayoutToImageLayout(VkImageLayout layout);
	VkImageLayout ImageLayoutToVkImageLayout(ImageLayout layout);
	ImageFormat VkFormatToImageFormat(VkFormat format);
	VkFormat ImageFormatToVkFormat(ImageFormat format);

	FilterMode VkFilterToFilterMode(VkFilter filter);
	VkFilter FilterModeToVkFilter(FilterMode filter);
	AddressMode VkSamplerAddressModeToAddressMode(VkSamplerAddressMode mode);
	VkSamplerAddressMode AddressModeToVkSamplerAddressMode(AddressMode mode);
	MipmapMode VkSamplerMipmapModeToMipmapMode(VkSamplerMipmapMode mode);
	VkSamplerMipmapMode MipmapModeToVkSamplerMipmapMode(MipmapMode mode);

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanImage
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanImage
    {
    public:
        // Constructors & Destructor
        VulkanImage() = default;
        ~VulkanImage() = default;

		// Init & Destroy
        void Init(const RendererID renderer, const ImageSpecification& imageSpecs, const SamplerSpecification& samplerSpecs);
        void Init(const RendererID renderer, const ImageSpecification& imageSpecs, const SamplerSpecification& samplerSpecs, const std::filesystem::path& imagePath);
        void Init(const RendererID renderer, const ImageSpecification& imageSpecs, const VkImage image, const VkImageView imageView); // Note: This exists for swapchain images
        void Destroy(const RendererID renderer);

        // Methods
        void SetData(const RendererID renderer, void* data, size_t size);

        void Resize(const RendererID renderer, uint32_t width, uint32_t height);

        void Transition(const RendererID renderer, ImageLayout initial, ImageLayout final);

        // Getters
        inline const ImageSpecification& GetSpecification() const { return m_ImageSpecification; }
		inline const SamplerSpecification& GetSamplerSpecification() const { return m_SamplerSpecification; }

        inline uint32_t GetWidth() const { return m_ImageSpecification.Width; }
        inline uint32_t GetHeight() const { return m_ImageSpecification.Height; }

        // Internal getters
        inline VkImage GetVkImage() const { return m_Image; }
        inline VmaAllocation GetVmaAllocation() const { return m_Allocation; }
        inline VkImageView GetVkImageView() const { return m_ImageView; }
        inline VkSampler GetVkSampler() const { return m_Sampler; }

    private:
        // Private methods
        void CreateImage(const RendererID renderer, uint32_t width, uint32_t height);
        void CreateImage(const RendererID renderer, const std::filesystem::path& imagePath);
        void GenerateMipmaps(const RendererID renderer, VkImage& image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
        void DestroyImage(const RendererID renderer);

    private:
        ImageSpecification m_ImageSpecification = {};
        SamplerSpecification m_SamplerSpecification = {};

        VkImage m_Image = VK_NULL_HANDLE;
        VmaAllocation m_Allocation = VK_NULL_HANDLE;
        VkImageView m_ImageView = VK_NULL_HANDLE;
        VkSampler m_Sampler = VK_NULL_HANDLE;

        uint32_t m_Miplevels = 1;

        friend class VulkanSwapChain;
        friend class VulkanDescriptorSet;
    };

}