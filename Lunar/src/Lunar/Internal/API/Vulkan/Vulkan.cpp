#include "lupch.h"
#include "Vulkan.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanRenderer.hpp"

#include <cstring>

namespace Lunar::Internal
{

	namespace Vk
	{

		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

			if (func != nullptr)
				return func(instance, pCreateInfo, pAllocator, pDebugMessenger);

			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

			if (func != nullptr)
				func(instance, debugMessenger, pAllocator);
		}

		VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
		{
			if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			{
				LU_LOG_WARN("Validation layer: {0}", pCallbackData->pMessage);
				return VK_FALSE;
			}

			return VK_FALSE;
		}

		bool ValidationLayersSupported()
		{
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

			// Check if all requested layers are actually accessible
			for (const char* layerName : g_VkRequestedValidationLayers)
			{
				bool layerFound = false;

				for (const auto& layerProperties : availableLayers)
				{
					if (strcmp(layerName, layerProperties.layerName) == 0)
					{
						layerFound = true;
						break;
					}
				}

				if (!layerFound)
					return false;
			}

			return true;
		}

	}

	VulkanCommand::VulkanCommand(RendererID renderer, bool start)
		: m_Renderer(renderer)
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = VulkanRenderer::GetRenderer(m_Renderer).GetVulkanSwapChain().GetVkCommandPool();
		allocInfo.commandBufferCount = 1;

		VK_VERIFY(vkAllocateCommandBuffers(VulkanContext::GetVulkanDevice().GetVkDevice(), &allocInfo, &m_CommandBuffer));

		if (start)
			Begin();
	}

	VulkanCommand::~VulkanCommand()
	{
		vkFreeCommandBuffers(VulkanContext::GetVulkanDevice().GetVkDevice(), VulkanRenderer::GetRenderer(m_Renderer).GetVulkanSwapChain().GetVkCommandPool(), 1, &m_CommandBuffer);
	}

	void VulkanCommand::Begin()
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);
	}

	void VulkanCommand::End()
	{
		vkEndCommandBuffer(m_CommandBuffer);
	}

	void VulkanCommand::Submit()
	{
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;

		auto queue = VulkanContext::GetVulkanDevice().GetGraphicsQueue();
		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);
	}

	void VulkanCommand::EndAndSubmit()
	{
		End();
		Submit();
	}

}