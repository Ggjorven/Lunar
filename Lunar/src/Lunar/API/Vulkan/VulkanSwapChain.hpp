#pragma once

#include <cstdint>

#include "Lunar/Core/Window.hpp"

#include "Lunar/API/Vulkan/Vulkan.hpp"
#include "Lunar/API/Vulkan/VulkanImage.hpp"

namespace Lunar
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Vulkan SwapChain
    ////////////////////////////////////////////////////////////////////////////////////
	class VulkanSwapChain
    {
    public:
        // Constructors & Destructor
        VulkanSwapChain(Window& window);
        ~VulkanSwapChain();

        // Methods
        void Init(uint32_t width, uint32_t height, bool vsync, uint8_t framesInFlight);

        // Getters
        inline const VkFormat GetColourFormat() const { return m_ColourFormat; }

        inline uint32_t GetCurrentFrame() const { return m_CurrentFrame; }
        inline uint32_t GetAquiredImage() const { return m_AcquiredImage; }

        inline std::vector<Arc<VulkanImage>>& GetSwapChainImages() { return m_Images; }
        inline Arc<VulkanImage> GetDepthImage() { return m_DepthStencil; }

        inline const VkSurfaceKHR GetVkSurface() const { return m_Surface; }
        inline const VkCommandPool GetVkCommandPool() const { return m_CommandPool; }

        inline const VkSemaphore GetImageAvailableSemaphore(uint32_t index) const { return m_ImageAvailableSemaphores[index]; }
        inline const VkSemaphore GetCurrentImageAvailableSemaphore() const { return GetImageAvailableSemaphore(m_CurrentFrame); }

        // Static methods
        static Arc<VulkanSwapChain> Create(Window& window);

    private:
        // Private methods
        uint32_t AcquireNextImage();
        void FindImageFormatAndColorSpace();

    private:
        Window& m_Window;

        VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

        std::vector<Arc<VulkanImage>> m_Images = { };
        Arc<VulkanImage> m_DepthStencil = nullptr;

        VkCommandPool m_CommandPool = VK_NULL_HANDLE;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores = { };

        VkFormat m_ColourFormat = VK_FORMAT_UNDEFINED;
        VkColorSpaceKHR m_ColourSpace = VK_COLOR_SPACE_MAX_ENUM_KHR;

        uint32_t m_CurrentFrame = 0;
        uint32_t m_AcquiredImage = 0;
    };

}