#pragma once

#include "Lunar/API/Vulkan/Vulkan.hpp"

#include "Lunar/Renderer/Renderer.hpp"

namespace Lunar
{

    class VulkanSwapChain;

    ////////////////////////////////////////////////////////////////////////////////////
    // Internal structs
    ////////////////////////////////////////////////////////////////////////////////////
    struct VulkanImageSpecification
    {
    public:
	    uint32_t Width = 0;
	    uint32_t Height = 0;

        VkImageUsageFlags Flags = VK_IMAGE_USAGE_SAMPLED_BIT;
        VkImageLayout Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkFormat Format = VK_FORMAT_R8G8B8A8_UNORM;

        bool MipMaps = true;
    };

    struct VulkanSamplerSpecification
    {
    public:
        VkFilter MagFilter = VK_FILTER_LINEAR;
        VkFilter MinFilter = VK_FILTER_LINEAR;
        VkSamplerAddressMode AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT; // For U, V & W
        VkSamplerMipmapMode MipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanImage
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanImage /* : public Image */
	{
	public:
        // Constructors & Destructor
		VulkanImage(RendererID renderer, const VulkanImageSpecification& imageSpecs, const VulkanSamplerSpecification& samplerSpecs);
        VulkanImage(RendererID renderer, const VulkanImageSpecification& imageSpecs, const VkImage image, const VkImageView imageView); // Note: This exists for swapchain images
		~VulkanImage();

        // Methods
		void SetData(void* data, size_t size) /* override */;

		void Resize(uint32_t width, uint32_t height) /* override */;

		void Transition(VkImageLayout initial, VkImageLayout final) /* override */;

        // Getters
        inline const VulkanImageSpecification& GetSpecification() const { return m_ImageSpecification; }

		inline uint32_t GetWidth() const { return m_ImageSpecification.Width; }
        inline uint32_t GetHeight() const { return m_ImageSpecification.Height; }

        // Internal getters
		inline const VkImage GetVkImage() const { return m_Image; }
		inline const VmaAllocation GetVmaAllocation() const { return m_Allocation; }
		inline const VkImageView GetVkImageView() const { return m_ImageView; }
		inline const VkSampler GetVkSampler() const { return m_Sampler; }

    private:
        // Private methods
        void CreateImage(uint32_t width, uint32_t height);
		void GenerateMipmaps(VkImage& image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
        void Destroy();

	private:
        const RendererID m_Renderer;
		VulkanImageSpecification m_ImageSpecification;
        VulkanSamplerSpecification m_SamplerSpecification;

		VkImage m_Image = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
		VkImageView m_ImageView = VK_NULL_HANDLE;
		VkSampler m_Sampler = VK_NULL_HANDLE;

		uint32_t m_Miplevels = 1;

        friend class VulkanSwapChain;
	};

}