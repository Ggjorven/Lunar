#pragma once

#include <cstdint>

#include "Lunar/API/Vulkan/Vulkan.hpp"
#include "Lunar/API/Vulkan/VulkanRenderer.hpp"

namespace Lunar
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Internal Vulkan Allocator
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanAllocator
    {
    public:
        // Initialization & Destroy methods
        static void Init();
        static void Destroy();

        // Setters
        // Note: This function is not used anywhere, but I like giving the user the option
        static void SetPipelineCache(const std::vector<uint8_t>& data); 

    public:
        // Buffers
        static VmaAllocation AllocateBuffer(RendererID rendererID, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer& dstBuffer, VkMemoryPropertyFlags requiredFlags = 0);
		static void CopyBuffer(RendererID rendererID, VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size, VkDeviceSize offset = 0);
        static void DestroyBuffer(RendererID rendererID, VkBuffer buffer, VmaAllocation allocation);

        // Image
        static VmaAllocation AllocateImage(RendererID rendererID, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memUsage, VkImage& image, VkMemoryPropertyFlags requiredFlags = {});
		static void CopyBufferToImage(RendererID rendererID, VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height);
		static VkImageView CreateImageView(RendererID rendererID, VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
		static VkSampler CreateSampler(RendererID rendererID, VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressmode, VkSamplerMipmapMode mipmapMode, uint32_t mipLevels);
        static void DestroyImage(RendererID rendererID, VkImage image, VmaAllocation allocation);

        // Utils
        static void MapMemory(VmaAllocation& allocation, void*& mapData);
		static void UnMapMemory(VmaAllocation& allocation);
        static void* SetData(VmaAllocation& allocation, void* data, size_t size);

    public:
        inline static VmaAllocator s_Allocator = VK_NULL_HANDLE;
        inline static VkPipelineCache s_PipelineCache = VK_NULL_HANDLE;
    };
}