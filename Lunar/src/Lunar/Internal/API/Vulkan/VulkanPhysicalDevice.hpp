#pragma once

#include <cstdint>
#include <optional>

#include "Lunar/Internal/API/Vulkan/Vulkan.hpp"

namespace Lunar::Internal
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
        VulkanPhysicalDevice() = default;
        ~VulkanPhysicalDevice() = default;

		// Init
		void Init(const VkSurfaceKHR surface);
		// Note: There is no destroy method since we didn't create the physical device, just select it.

        // Methods
        VkFormat FindDepthFormat() const;
        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

        // Getters
        inline VkPhysicalDevice GetVkPhysicalDevice() const { return m_PhysicalDevice; }
        
    private:
        // Private methods
        bool PhysicalDeviceSuitable(const VkSurfaceKHR surface, const VkPhysicalDevice device);
        bool ExtensionsSupported(const VkPhysicalDevice device);

        bool FeaturesSupported(const VkPhysicalDeviceFeatures& requested, const VkPhysicalDeviceFeatures& found);

    private:
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    };

}