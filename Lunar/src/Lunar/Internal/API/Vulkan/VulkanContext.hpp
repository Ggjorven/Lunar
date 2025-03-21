#pragma once

#include "Lunar/Internal/API/Vulkan/Vulkan.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanDevice.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanPhysicalDevice.hpp"

namespace Lunar::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanContext
    ////////////////////////////////////////////////////////////////////////////////////
	class VulkanContext
	{
    public:
        // Constructors & Destructor
        VulkanContext() = default;
        ~VulkanContext() = default;

		// Init & Destroy
		void Init(void* window);
		void Destroy();

        // Static getters
        static VulkanDevice& GetVulkanDevice();
        static VulkanPhysicalDevice& GetVulkanPhysicalDevice();

        static const VkInstance GetVkInstance();
        static const VkDebugUtilsMessengerEXT GetVkDebugger();

    private:
        // Private methods
        void InitInstance();
        void InitDevices(void* window);

	private:
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

        VulkanDevice m_Device = {};
        VulkanPhysicalDevice m_PhysicalDevice = {};
	};

}