#include "lupch.h"
#include "VulkanDevice.hpp"

#include "Lunar/Internal/IO/Print.hpp"

namespace Lunar::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// Init & Destroy
	////////////////////////////////////////////////////////////////////////////////////
	void VulkanDevice::Init(const VkSurfaceKHR surface, VulkanPhysicalDevice& physicalDevice)
	{
		m_PhysicalDevice = &physicalDevice;

		QueueFamilyIndices indices = QueueFamilyIndices::Find(surface, m_PhysicalDevice->GetVkPhysicalDevice());

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo& queueCreateInfo = queueCreateInfos.emplace_back();
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
		}

		// Enable dynamic rendering features
		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature = {};
		dynamicRenderingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
		dynamicRenderingFeature.dynamicRendering = VK_TRUE;

		// Enable descriptor indexing features (for bindless support)
		VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures = {};
		indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
		indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
		indexingFeatures.runtimeDescriptorArray = VK_TRUE;
		indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
		indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;

		// Chain both features into the pNext chain
		indexingFeatures.pNext = &dynamicRenderingFeature;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = &indexingFeatures; // Chain indexing & dynamic rendering
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &g_VkRequestedDeviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(g_VkRequestedDeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = g_VkRequestedDeviceExtensions.data();

		if constexpr (g_VkValidation)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(g_VkRequestedValidationLayers.size());
			createInfo.ppEnabledLayerNames = g_VkRequestedValidationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		VK_VERIFY(vkCreateDevice(m_PhysicalDevice->GetVkPhysicalDevice(), &createInfo, nullptr, &m_LogicalDevice));

		// Retrieve the graphics/compute/present queue handle
		vkGetDeviceQueue(m_LogicalDevice, indices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_LogicalDevice, indices.ComputeFamily.value(), 0, &m_ComputeQueue);
		vkGetDeviceQueue(m_LogicalDevice, indices.PresentFamily.value(), 0, &m_PresentQueue);
	}

	void VulkanDevice::Destroy()
	{
		vkDestroyDevice(m_LogicalDevice, nullptr);
	}

    ////////////////////////////////////////////////////////////////////////////////////
	// Methods
    ////////////////////////////////////////////////////////////////////////////////////
	void VulkanDevice::Wait() const
	{
		vkDeviceWaitIdle(m_LogicalDevice);
	}

}