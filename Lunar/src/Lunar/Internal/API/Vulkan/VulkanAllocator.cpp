#include "lupch.h"
#include "VulkanAllocator.hpp"

#include "Lunar/Internal/IO/Print.hpp"
#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"

#if defined(LU_COMPILER_GCC)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-variable"
    #pragma GCC diagnostic ignored "-Wunused-parameter"
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

    #pragma GCC diagnostic pop
#elif !defined(LU_PLATFORM_APPLE)
    #pragma warning(push, 0)

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

    #pragma warning(pop)
#endif

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Callbacks
    ////////////////////////////////////////////////////////////////////////////////////
    static void* VKAPI_PTR VmaAllocFn(void*, size_t size, size_t, VkSystemAllocationScope)
    {
        return std::malloc(size);
    }

    static void VKAPI_PTR VmaFreeFn(void*, void* pMemory)
    {
        std::free(pMemory);
    }

    static void* VKAPI_PTR VmaReallocFn(void*, void* pOriginal, size_t size, size_t, VkSystemAllocationScope)
    {
        return std::realloc(pOriginal, size);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Initialization & Destroy methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanAllocator::Init()
    {
        VkAllocationCallbacks callbacks = {};
        callbacks.pUserData = nullptr;
        callbacks.pfnAllocation = &VmaAllocFn;
        callbacks.pfnFree = &VmaFreeFn;
        callbacks.pfnReallocation = &VmaReallocFn;
        callbacks.pfnInternalAllocation = nullptr;
        callbacks.pfnInternalFree = nullptr;

        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.instance = VulkanContext::GetVkInstance();
        allocatorInfo.physicalDevice = VulkanContext::GetVulkanPhysicalDevice().GetVkPhysicalDevice();
        allocatorInfo.device = VulkanContext::GetVulkanDevice().GetVkDevice();
        allocatorInfo.pAllocationCallbacks = &callbacks;

        VK_VERIFY(vmaCreateAllocator(&allocatorInfo, &s_Allocator));
    }

    void VulkanAllocator::Destroy()
    {
        vmaDestroyAllocator(s_Allocator);
        s_Allocator = VK_NULL_HANDLE;
    }

    void VulkanAllocator::SetPipelineCache(const std::vector<uint8_t>& data)
    {
        VkPipelineCacheCreateInfo cacheCreateInfo = {};
        cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        cacheCreateInfo.initialDataSize = data.size();
        cacheCreateInfo.pInitialData = data.data();

        VK_VERIFY(vkCreatePipelineCache(VulkanContext::GetVulkanDevice().GetVkDevice(), &cacheCreateInfo, nullptr, &s_PipelineCache));
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Buffers
    ////////////////////////////////////////////////////////////////////////////////////
    VmaAllocation VulkanAllocator::AllocateBuffer(const RendererID, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer& dstBuffer, VkMemoryPropertyFlags requiredFlags)
    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Change if necessary

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = memoryUsage; // VMA_MEMORY_USAGE_GPU_ONLY, VMA_MEMORY_USAGE_CPU_ONLY, etc.
        allocInfo.requiredFlags = requiredFlags;

        VmaAllocation allocation = VK_NULL_HANDLE;
        VK_VERIFY(vmaCreateBuffer(s_Allocator, &bufferInfo, &allocInfo, &dstBuffer, &allocation, nullptr));

        return allocation;
    }

    void VulkanAllocator::CopyBuffer(const RendererID rendererID, VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size, VkDeviceSize offset)
    {
        VulkanCommand command = VulkanCommand(rendererID, true);

        VkBufferCopy copyRegion = {};
        copyRegion.size = size;
        copyRegion.dstOffset = offset;
        vkCmdCopyBuffer(command.GetVkCommandBuffer(), srcBuffer, dstBuffer, 1, &copyRegion);

        command.EndAndSubmit();
    }

    void VulkanAllocator::DestroyBuffer(const RendererID, VkBuffer buffer, VmaAllocation allocation)
    {
        vmaDestroyBuffer(s_Allocator, buffer, allocation);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Image
    ////////////////////////////////////////////////////////////////////////////////////
    VmaAllocation VulkanAllocator::AllocateImage(const RendererID, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memUsage, VkImage& image, VkMemoryPropertyFlags requiredFlags)
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = memUsage;
        allocCreateInfo.requiredFlags = requiredFlags;

        VmaAllocation allocation = VK_NULL_HANDLE;
        VK_VERIFY(vmaCreateImage(s_Allocator, &imageInfo, &allocCreateInfo, &image, &allocation, nullptr));

        return allocation;
    }

    void VulkanAllocator::CopyBufferToImage(const RendererID rendererID, VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height)
    {
        VulkanCommand command = VulkanCommand(rendererID, true);

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(command.GetVkCommandBuffer(), buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        command.EndAndSubmit();
    }

    VkImageView VulkanAllocator::CreateImageView(const RendererID, VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.aspectMask = aspectFlags;

        VkImageView imageView = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateImageView(VulkanContext::GetVulkanDevice().GetVkDevice(), &viewInfo, nullptr, &imageView));

        return imageView;
    }

    VkSampler VulkanAllocator::CreateSampler(const RendererID, VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressmode, VkSamplerMipmapMode mipmapMode, uint32_t mipLevels)
    {
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = magFilter;
        samplerInfo.minFilter = minFilter;
        samplerInfo.addressModeU = addressmode;
        samplerInfo.addressModeV = addressmode;
        samplerInfo.addressModeW = addressmode;

        VkPhysicalDeviceProperties properties = {};
        vkGetPhysicalDeviceProperties(VulkanContext::GetVulkanPhysicalDevice().GetVkPhysicalDevice(), &properties);

        samplerInfo.anisotropyEnable = VK_TRUE;                             // Can be disabled: just set VK_FALSE
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // And 1.0f

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = mipmapMode;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevels);
        samplerInfo.mipLodBias = 0.0f; // Optional

        VkSampler sampler = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateSampler(VulkanContext::GetVulkanDevice().GetVkDevice(), &samplerInfo, nullptr, &sampler));

        return sampler;
    }

    void VulkanAllocator::DestroyImage(const RendererID, VkImage image, VmaAllocation allocation)
    {
        vmaDestroyImage(s_Allocator, image, allocation);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Utils
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanAllocator::MapMemory(VmaAllocation& allocation, void*& mapData)
    {
        vmaMapMemory(s_Allocator, allocation, &mapData);
    }

    void VulkanAllocator::UnMapMemory(VmaAllocation& allocation)
    {
        vmaUnmapMemory(s_Allocator, allocation);
    }

    void* VulkanAllocator::SetData(VmaAllocation& allocation, void* data, size_t size)
    {
        void* mappedData;

        VulkanAllocator::MapMemory(allocation, mappedData);
        memcpy(mappedData, data, size);
        VulkanAllocator::UnMapMemory(allocation);

        return mappedData;
    }


}