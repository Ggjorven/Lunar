#include "lupch.h"
#include "VulkanPhysicalDevice.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"

#include <tuple>
#include <ranges>

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Internal structs
    ////////////////////////////////////////////////////////////////////////////////////
    QueueFamilyIndices QueueFamilyIndices::Find(const VkSurfaceKHR surface, const VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = {};

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int32_t i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			// Early exit check
			if (indices.IsComplete())
				break;

			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.GraphicsFamily = i;

			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
				indices.ComputeFamily = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
				indices.PresentFamily = i;

			i++;
		}

		return indices;
	}

	SwapChainSupportDetails SwapChainSupportDetails::Query(const VkSurfaceKHR surface, const VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		// Capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);

		// Formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.Formats.resize(static_cast<size_t>(formatCount));
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.Formats.data());
		}

		// Presentation modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.PresentModes.resize(static_cast<size_t>(presentModeCount));
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentModes.data());
		}

		return details;
	}

    ////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanPhysicalDevice::Init(const VkSurfaceKHR surface)
    {
        const VkInstance instance = VulkanContext::GetVkInstance();

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        LU_ASSERT(!(deviceCount == 0), "[VulkanPhysicalDevice] Failed to find a GPU with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) 
		{
            if (PhysicalDeviceSuitable(surface, device)) 
			{
                m_PhysicalDevice = device;
                break;
            }
        }

        LU_ASSERT(m_PhysicalDevice, "[VulkanPhysicalDevice] Failed to find a GPU with support for this application's required Vulkan capabilities!");
    }

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
    ////////////////////////////////////////////////////////////////////////////////////
    VkFormat VulkanPhysicalDevice::FindDepthFormat() const
    {
        return FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT
        );
    }

    VkFormat VulkanPhysicalDevice::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
    {
        for (const auto& format : candidates) 
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                return format;
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                return format;
        }

        LU_LOG_ERROR("[VulkanPhysicalDevice] Failed to find supported format!");
        return VK_FORMAT_UNDEFINED;
    }

	////////////////////////////////////////////////////////////////////////////////////
    // Private methods
	////////////////////////////////////////////////////////////////////////////////////
	bool VulkanPhysicalDevice::PhysicalDeviceSuitable(const VkSurfaceKHR surface, const VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = QueueFamilyIndices::Find(surface, device);

		bool extensionsSupported = ExtensionsSupported(device);
		bool swapChainAdequate = false;

		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = SwapChainSupportDetails::Query(surface, device);
			swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
		}

        VkPhysicalDeviceFeatures supportedFeatures = {};
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		// Index features
        VkPhysicalDeviceDescriptorIndexingFeatures indexFeatures = {};
		indexFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		indexFeatures.pNext = nullptr;

        VkPhysicalDeviceFeatures2 deviceFeatures = {};
		deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures.pNext = &indexFeatures;

		vkGetPhysicalDeviceFeatures2(device, &deviceFeatures);

		// Check support for bindless textures
		bool bindlessSupport = indexFeatures.descriptorBindingPartiallyBound && indexFeatures.runtimeDescriptorArray;

		return indices.IsComplete() && extensionsSupported && swapChainAdequate && FeaturesSupported(g_VkRequestedDeviceFeatures, supportedFeatures) && bindlessSupport;
	}

	bool VulkanPhysicalDevice::ExtensionsSupported(const VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(g_VkRequestedDeviceExtensions.begin(), g_VkRequestedDeviceExtensions.end());

		for (const auto& extension : availableExtensions)
			requiredExtensions.erase(extension.extensionName);

		// Note: It's empty if all the required extensions are available
		return requiredExtensions.empty();
	}

	bool VulkanPhysicalDevice::FeaturesSupported(const VkPhysicalDeviceFeatures& requested, const VkPhysicalDeviceFeatures& found)
    {
        constexpr auto features = std::tuple{ 
			&VkPhysicalDeviceFeatures::robustBufferAccess,
            &VkPhysicalDeviceFeatures::fullDrawIndexUint32,
            &VkPhysicalDeviceFeatures::imageCubeArray,
            &VkPhysicalDeviceFeatures::independentBlend,
            &VkPhysicalDeviceFeatures::geometryShader,
            &VkPhysicalDeviceFeatures::tessellationShader,
            &VkPhysicalDeviceFeatures::sampleRateShading,
            &VkPhysicalDeviceFeatures::dualSrcBlend,
            &VkPhysicalDeviceFeatures::logicOp,
            &VkPhysicalDeviceFeatures::multiDrawIndirect,
            &VkPhysicalDeviceFeatures::drawIndirectFirstInstance,
            &VkPhysicalDeviceFeatures::depthClamp,
            &VkPhysicalDeviceFeatures::depthBiasClamp,
            &VkPhysicalDeviceFeatures::fillModeNonSolid,
            &VkPhysicalDeviceFeatures::depthBounds,
            &VkPhysicalDeviceFeatures::wideLines,
            &VkPhysicalDeviceFeatures::largePoints,
            &VkPhysicalDeviceFeatures::alphaToOne,
            &VkPhysicalDeviceFeatures::multiViewport,
            &VkPhysicalDeviceFeatures::samplerAnisotropy,
            &VkPhysicalDeviceFeatures::textureCompressionETC2,
            &VkPhysicalDeviceFeatures::textureCompressionASTC_LDR,
            &VkPhysicalDeviceFeatures::textureCompressionBC,
            &VkPhysicalDeviceFeatures::occlusionQueryPrecise,
            &VkPhysicalDeviceFeatures::pipelineStatisticsQuery,
            &VkPhysicalDeviceFeatures::vertexPipelineStoresAndAtomics,
            &VkPhysicalDeviceFeatures::fragmentStoresAndAtomics,
            &VkPhysicalDeviceFeatures::shaderTessellationAndGeometryPointSize,
            &VkPhysicalDeviceFeatures::shaderImageGatherExtended,
            &VkPhysicalDeviceFeatures::shaderStorageImageExtendedFormats,
            &VkPhysicalDeviceFeatures::shaderStorageImageMultisample,
            &VkPhysicalDeviceFeatures::shaderStorageImageReadWithoutFormat,
            &VkPhysicalDeviceFeatures::shaderStorageImageWriteWithoutFormat,
            &VkPhysicalDeviceFeatures::shaderUniformBufferArrayDynamicIndexing,
            &VkPhysicalDeviceFeatures::shaderSampledImageArrayDynamicIndexing,
            &VkPhysicalDeviceFeatures::shaderStorageBufferArrayDynamicIndexing,
            &VkPhysicalDeviceFeatures::shaderStorageImageArrayDynamicIndexing,
            &VkPhysicalDeviceFeatures::shaderClipDistance,
            &VkPhysicalDeviceFeatures::shaderCullDistance,
            &VkPhysicalDeviceFeatures::shaderFloat64,
            &VkPhysicalDeviceFeatures::shaderInt64,
            &VkPhysicalDeviceFeatures::shaderInt16,
            &VkPhysicalDeviceFeatures::shaderResourceResidency,
            &VkPhysicalDeviceFeatures::shaderResourceMinLod,
            &VkPhysicalDeviceFeatures::sparseBinding,
            &VkPhysicalDeviceFeatures::sparseResidencyBuffer,
            &VkPhysicalDeviceFeatures::sparseResidencyImage2D,
            &VkPhysicalDeviceFeatures::sparseResidencyImage3D,
            &VkPhysicalDeviceFeatures::sparseResidency2Samples,
            &VkPhysicalDeviceFeatures::sparseResidency4Samples,
            &VkPhysicalDeviceFeatures::sparseResidency8Samples,
            &VkPhysicalDeviceFeatures::sparseResidency16Samples,
            &VkPhysicalDeviceFeatures::sparseResidencyAliased,
            &VkPhysicalDeviceFeatures::variableMultisampleRate,
            &VkPhysicalDeviceFeatures::inheritedQueries 
		};

        // Use std::apply to iterate over the tuple
        bool failed = false;
        std::apply([&](auto... featurePtr) { ((failed |= (requested.*featurePtr && !(found.*featurePtr))), ...); }, features);

        return !failed;
    }

}