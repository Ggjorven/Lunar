#pragma once

#include "Lunar/API/Vulkan/Vulkan.hpp"

#include "Lunar/API/Vulkan/VulkanPhysicalDevice.hpp"

namespace Lunar
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Vulkan Logical Device
    ////////////////////////////////////////////////////////////////////////////////////
	class VulkanDevice
    {
    public:
        // Constructor & Destructor
        VulkanDevice(const VkSurfaceKHR surface, Arc<VulkanPhysicalDevice> physicalDevice);
        ~VulkanDevice();

        // Methods
        void Wait() const;

        // Getters
        inline const VkDevice GetVkDevice() const { return m_LogicalDevice; }

        inline const VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
        inline const VkQueue GetComputeQueue() const { return m_ComputeQueue; }
        inline const VkQueue GetPresentQueue() const { return m_PresentQueue; }

        inline Arc<VulkanPhysicalDevice> GetPhysicalDevice() const { return m_PhysicalDevice; }

        // Static methods
        static Arc<VulkanDevice> Create(const VkSurfaceKHR surface, Arc<VulkanPhysicalDevice> physicalDevice);

    private:
        Arc<VulkanPhysicalDevice> m_PhysicalDevice;
        VkDevice m_LogicalDevice = VK_NULL_HANDLE;

        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_ComputeQueue = VK_NULL_HANDLE;
        VkQueue m_PresentQueue = VK_NULL_HANDLE;
    };

}