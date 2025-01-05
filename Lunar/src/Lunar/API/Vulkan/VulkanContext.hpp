#pragma once

#include "Lunar/Memory/Arc.hpp"

#include "Lunar/Renderer/GraphicsContext.hpp"

#include "Lunar/API/Vulkan/Vulkan.hpp"
#include "Lunar/API/Vulkan/VulkanDevice.hpp"
#include "Lunar/API/Vulkan/VulkanPhysicalDevice.hpp"

namespace Lunar
{

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanContext
    ////////////////////////////////////////////////////////////////////////////////////
	class VulkanContext : public GraphicsContext
	{
    public:
        // Constructors & Destructor
        VulkanContext(std::vector<void*>& windows);
        ~VulkanContext();

        // Static getters
        inline static Arc<VulkanDevice> GetVulkanDevice() { return s_Instance->m_Device; }
        inline static Arc<VulkanPhysicalDevice> GetVulkanPhysicalDevice() { return s_Instance->m_PhysicalDevice; }

        inline static const VkInstance GetVkInstance() { return s_Instance->m_Instance; }
        inline static const VkDebugUtilsMessengerEXT GetVkDebugger() { return s_Instance->m_DebugMessenger; }

    private:
        // Private methods
        void InitInstance();
        void InitDevices(void* window);

	private:
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

        Arc<VulkanDevice> m_Device = nullptr;
        Arc<VulkanPhysicalDevice> m_PhysicalDevice = nullptr;

    private:
        inline static VulkanContext* s_Instance = nullptr;
	};

}