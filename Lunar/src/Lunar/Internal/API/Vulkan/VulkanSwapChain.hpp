#pragma once

#include <cstdint>

#include "Lunar/Internal/API/Vulkan/Vulkan.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanImage.hpp"

namespace Lunar::Internal
{

    class Window;
    class VulkanRenderer;
    
    ////////////////////////////////////////////////////////////////////////////////////
    // Vulkan SwapChain
    ////////////////////////////////////////////////////////////////////////////////////
	class VulkanSwapChain
    {
    public:
        // Constructors & Destructor
        VulkanSwapChain() = default;
        ~VulkanSwapChain() = default;

        // Init & Destroy
		void Init(Window* window);
		void Destroy();

        // Methods
        void Resize(uint32_t width, uint32_t height, bool vsync, uint8_t framesInFlight);

        // Getters
        inline const VkFormat GetColourFormat() const { return m_ColourFormat; }

        inline uint32_t GetCurrentFrame() const { return m_CurrentFrame; }
        inline uint32_t GetAquiredImage() const { return m_AcquiredImage; }

        inline std::vector<VulkanImage>& GetSwapChainImages() { return m_Images; }
        inline VulkanImage& GetDepthImage() { return m_DepthStencil; }

        inline const VkSurfaceKHR GetVkSurface() const { return m_Surface; }
        inline const VkCommandPool GetVkCommandPool() const { return m_CommandPool; }

        inline const VkSemaphore GetImageAvailableSemaphore(uint32_t index) const { return m_ImageAvailableSemaphores[index]; }
        inline const VkSemaphore GetCurrentImageAvailableSemaphore() const { return GetImageAvailableSemaphore(m_CurrentFrame); }

    private:
        // Private methods
        uint32_t AcquireNextImage();
        void FindImageFormatAndColorSpace();

    private:
        Window* m_Window = nullptr;

        VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

        std::vector<VulkanImage> m_Images = { };
        VulkanImage m_DepthStencil = {};

        VkCommandPool m_CommandPool = VK_NULL_HANDLE;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores = { };

        VkFormat m_ColourFormat = VK_FORMAT_UNDEFINED;
        VkColorSpaceKHR m_ColourSpace = VK_COLOR_SPACE_MAX_ENUM_KHR;

        uint32_t m_CurrentFrame = 0;
        uint32_t m_AcquiredImage = 0;

        friend class VulkanRenderer;
    };

}