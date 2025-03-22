#pragma once

#include "Lunar/Internal/API/Vulkan/Vulkan.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanPhysicalDevice.hpp"

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Vulkan Logical Device
    ////////////////////////////////////////////////////////////////////////////////////
	class VulkanDevice
    {
    public:
        // Constructor & Destructor
        VulkanDevice() = default;
        ~VulkanDevice() = default;

		// Init & Destroy
		void Init(const VkSurfaceKHR surface, VulkanPhysicalDevice& physicalDevice);
        void Destroy();
        
        // Methods
        void Wait() const;

        // Getters
        inline VkDevice GetVkDevice() const { return m_LogicalDevice; }

        inline VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
        inline VkQueue GetComputeQueue() const { return m_ComputeQueue; }
        inline VkQueue GetPresentQueue() const { return m_PresentQueue; }

        inline VulkanPhysicalDevice& GetPhysicalDevice() const { return *m_PhysicalDevice; }

    private:
        VulkanPhysicalDevice* m_PhysicalDevice = nullptr;
        VkDevice m_LogicalDevice = VK_NULL_HANDLE;

        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_ComputeQueue = VK_NULL_HANDLE;
        VkQueue m_PresentQueue = VK_NULL_HANDLE;
    };

}