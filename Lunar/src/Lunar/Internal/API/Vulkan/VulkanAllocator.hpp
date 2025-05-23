#pragma once

#include <cstdint>

#include "Lunar/Internal/API/Vulkan/Vulkan.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanRenderer.hpp"

namespace Lunar::Internal
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
        static VmaAllocation AllocateBuffer(const RendererID rendererID, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer& dstBuffer, VkMemoryPropertyFlags requiredFlags = 0);
        static void CopyBuffer(const RendererID rendererID, VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size, VkDeviceSize offset = 0);
        static void DestroyBuffer(const RendererID rendererID, VkBuffer buffer, VmaAllocation allocation);

        // Image
        static VmaAllocation AllocateImage(const RendererID rendererID, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memUsage, VkImage& image, VkMemoryPropertyFlags requiredFlags = {});
        static void CopyBufferToImage(const RendererID rendererID, VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height);
        static VkImageView CreateImageView(const RendererID rendererID, VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
        static VkSampler CreateSampler(const RendererID rendererID, VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressmode, VkSamplerMipmapMode mipmapMode, uint32_t mipLevels);
        static void DestroyImage(const RendererID rendererID, VkImage image, VmaAllocation allocation);

        // Utils
        static void MapMemory(VmaAllocation& allocation, void*& mapData);
        static void UnMapMemory(VmaAllocation& allocation);
        static void* SetData(VmaAllocation& allocation, void* data, size_t size);

    public:
        inline static VmaAllocator s_Allocator = VK_NULL_HANDLE;
        inline static VkPipelineCache s_PipelineCache = VK_NULL_HANDLE;
    };
}