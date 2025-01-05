#pragma once

#include <cstdint>
#include <optional>

#include "Lunar/API/Vulkan/Vulkan.hpp"

#include "Lunar/Memory/Arc.hpp"

namespace Lunar
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Internal structs
    ////////////////////////////////////////////////////////////////////////////////////
    struct QueueFamilyIndices
    {
    public:
        std::optional<uint32_t> GraphicsFamily;
        std::optional<uint32_t> ComputeFamily;
        std::optional<uint32_t> PresentFamily;

    public:
        static QueueFamilyIndices Find(const VkSurfaceKHR surface, const VkPhysicalDevice device);
        inline bool IsComplete() const { return GraphicsFamily.has_value() && ComputeFamily.has_value() && PresentFamily.has_value(); }
    };

    struct SwapChainSupportDetails
    {
    public:
        VkSurfaceCapabilitiesKHR Capabilities;
        std::vector<VkSurfaceFormatKHR> Formats;
        std::vector<VkPresentModeKHR> PresentModes;

    public:
        static SwapChainSupportDetails Query(const VkSurfaceKHR surface, const VkPhysicalDevice device);
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Vulkan Physical Device
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanPhysicalDevice
    {
    public:
        // Constructor & Destructor
        VulkanPhysicalDevice(const VkSurfaceKHR surface);
        ~VulkanPhysicalDevice();

        // Methods
        VkFormat FindDepthFormat();
        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        // Getters
        inline const VkPhysicalDevice GetVkPhysicalDevice() const { return m_PhysicalDevice; }
        
        // Static methods
        static Arc<VulkanPhysicalDevice> Select(const VkSurfaceKHR surface);

    private:
        // Private methods
        bool PhysicalDeviceSuitable(const VkSurfaceKHR surface, const VkPhysicalDevice device);
        bool ExtensionsSupported(const VkPhysicalDevice device);

        bool FeaturesSupported(const VkPhysicalDeviceFeatures& requested, const VkPhysicalDeviceFeatures& found);

    private:
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    };

}