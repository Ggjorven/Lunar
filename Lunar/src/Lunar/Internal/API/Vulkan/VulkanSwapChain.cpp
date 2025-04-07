#include "lupch.h"
#include "VulkanSwapChain.hpp"

#include "Lunar/Internal/IO/Print.hpp"
#include "Lunar/Internal/Utils/Profiler.hpp"

#include "Lunar/Internal/Core/Window.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanAllocator.hpp"

#if defined(LU_PLATFORM_DESKTOP)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
	// Init & Destroy
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanSwapChain::Init(const RendererID renderer, Window* window) 
	{
		m_RendererID = renderer;
		m_Window = window;

		#if defined(LU_PLATFORM_DESKTOP)
        VK_VERIFY(glfwCreateWindowSurface(VulkanContext::GetVkInstance(), static_cast<GLFWwindow*>(window->GetNativeWindow()), nullptr, &m_Surface));
		#endif

		FindImageFormatAndColorSpace();

		QueueFamilyIndices queueFamilyIndices = QueueFamilyIndices::Find(m_Surface, VulkanContext::GetVulkanPhysicalDevice().GetVkPhysicalDevice());

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allows us to reset the command buffer and reuse it.
		poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();

		VK_VERIFY(vkCreateCommandPool(VulkanContext::GetVulkanDevice().GetVkDevice(), &poolInfo, nullptr, &m_CommandPool));
	}

	void VulkanSwapChain::Destroy()
	{
        VulkanDevice& device = VulkanContext::GetVulkanDevice();
        device.Wait();

		VK_VERIFY(vkQueueWaitIdle(device.GetGraphicsQueue()));

		if (m_SwapChain)
            vkDestroySwapchainKHR(device.GetVkDevice(), m_SwapChain, nullptr);

		for (auto& image : m_Images)
			image.Destroy(m_RendererID);

		vkDestroyCommandPool(device.GetVkDevice(), m_CommandPool, nullptr);

		for (size_t i = 0; i < m_ImageAvailableSemaphores.size(); i++)
            vkDestroySemaphore(device.GetVkDevice(), m_ImageAvailableSemaphores[i], nullptr);

		vkDestroySurfaceKHR(VulkanContext::GetVkInstance(), m_Surface, nullptr);
	}

    ////////////////////////////////////////////////////////////////////////////////////
	// Methods
    ////////////////////////////////////////////////////////////////////////////////////
	void VulkanSwapChain::Resize(uint32_t width, uint32_t height, bool vsync, uint8_t framesInFlight)
	{
		if (width == 0 || height == 0)
            return;

		///////////////////////////////////////////////////////////
		// SwapChain
		///////////////////////////////////////////////////////////
		SwapChainSupportDetails details = SwapChainSupportDetails::Query(m_Surface, VulkanContext::GetVulkanPhysicalDevice().GetVkPhysicalDevice());

		VkExtent2D swapchainExtent = {};
		// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
        if (details.Capabilities.currentExtent.width == 0xFFFFFFFF)
		{
			// If the surface size is undefined, the size is set to
			// the size of the images requested.
			swapchainExtent.width = width;
			swapchainExtent.height = height;
		}
		else
		{
			// If the surface size is defined, the swap chain size must match
			swapchainExtent = details.Capabilities.currentExtent;
		}

		// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
		// This mode waits for the vertical blank ("v-sync")
		VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

		// If v-sync is not requested, try to find a mailbox mode
		// It's the lowest latency non-tearing present mode available
		if (!vsync)
		{
			for (size_t i = 0; i < details.PresentModes.size(); i++)
			{
				if (details.PresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
				if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (details.PresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
				{
					swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
			}
		}

		// Determine the number of images
		uint32_t desiredNumberOfSwapchainImages = details.Capabilities.minImageCount + 1;
		if ((details.Capabilities.maxImageCount > 0) && (desiredNumberOfSwapchainImages > details.Capabilities.maxImageCount))
		{
			desiredNumberOfSwapchainImages = details.Capabilities.maxImageCount;
		}

		// Find the transformation of the surface
		VkSurfaceTransformFlagsKHR preTransform;
		if (details.Capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		else
			preTransform = details.Capabilities.currentTransform;

		// Find a supported composite alpha format (not all devices support alpha opaque)
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// Simply select the first composite alpha format available
		std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto& compositeAlphaFlag : compositeAlphaFlags)
		{
			if (details.Capabilities.supportedCompositeAlpha & compositeAlphaFlag)
			{
				compositeAlpha = compositeAlphaFlag;
				break;
			}
		}

		VkSwapchainCreateInfoKHR swapchainCI = {};
		swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCI.pNext = NULL;
		swapchainCI.surface = m_Surface;
		swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
		swapchainCI.imageFormat = m_ColourFormat;
		swapchainCI.imageColorSpace = m_ColourSpace;
		swapchainCI.imageExtent = { swapchainExtent.width, swapchainExtent.height };

		swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (details.Capabilities.supportedTransforms & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (details.Capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
		swapchainCI.imageArrayLayers = 1;
		swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCI.queueFamilyIndexCount = 0;
		swapchainCI.pQueueFamilyIndices = NULL;
		swapchainCI.presentMode = swapchainPresentMode;
		swapchainCI.oldSwapchain = m_SwapChain;
		// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
		swapchainCI.clipped = VK_TRUE;
		swapchainCI.compositeAlpha = compositeAlpha;

		// Enable transfer source on swap chain images if supported
		if (details.Capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		// Enable transfer destination on swap chain images if supported
		if (details.Capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		auto device = VulkanContext::GetVulkanDevice().GetVkDevice();
		auto oldSwapchain = m_SwapChain;
		VK_VERIFY(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &m_SwapChain));

		if (oldSwapchain)
			vkDestroySwapchainKHR(device, oldSwapchain, nullptr); // Destroys images?

		// Get the swap chain images
		uint32_t imageCount = 0;
		std::vector<VkImage> tempImages = { };

		VK_VERIFY(vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, NULL));
		tempImages.resize(imageCount);
        VK_VERIFY(vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, tempImages.data()));

		if (m_Images.empty()) m_Images.resize((size_t)imageCount); // Make sure we can access the appropriate indices
		for (uint32_t i = 0; i < imageCount; i++)
		{
			VkImageViewCreateInfo colorAttachmentView = {};
			colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			colorAttachmentView.pNext = NULL;
			colorAttachmentView.format = m_ColourFormat;
			colorAttachmentView.image = tempImages[i];
			colorAttachmentView.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			colorAttachmentView.subresourceRange.baseMipLevel = 0;
			colorAttachmentView.subresourceRange.levelCount = 1;
			colorAttachmentView.subresourceRange.baseArrayLayer = 0;
			colorAttachmentView.subresourceRange.layerCount = 1;
			colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
			colorAttachmentView.flags = 0;

			VkImageView imageView = VK_NULL_HANDLE;
			VK_VERIFY(vkCreateImageView(device, &colorAttachmentView, nullptr, &imageView));

			ImageSpecification specs = {
				.Usage = ImageUsage::Colour,
                .Layout = ImageLayout::Undefined,
				.Format = VkFormatToImageFormat(m_ColourFormat),
                .Width = width,
                .Height = height,
                .MipMaps = false
			};

			if (m_Images[i].GetWidth() != 0 && m_Images[i].GetHeight() != 0)
			{
                VulkanImage& src = m_Images[i];

				// Destroy old image view
                m_Window->GetRenderer().GetInternalRenderer().Free([device = device, imageView = src.m_ImageView]()
				{
					vkDestroyImageView(device, imageView, nullptr);
				});

				// Set new data
				src.m_ImageSpecification = specs;
				src.m_Image = tempImages[i];
				src.m_ImageView = imageView;
			}
			else
                m_Images[i].Init(m_Window->GetRenderer().GetID(), specs, tempImages[i], imageView);

			// We transition manually since, the layout set in the specification doesn't get used
			// since we manually set all the data.
			m_Images[i].Transition(m_RendererID, ImageLayout::Undefined, ImageLayout::PresentSrcKHR);
		}

		///////////////////////////////////////////////////////////
		// Synchronization Objects
		///////////////////////////////////////////////////////////
		if (m_ImageAvailableSemaphores.empty())
		{
			m_ImageAvailableSemaphores.resize((size_t)framesInFlight);

			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkFenceCreateInfo fenceInfo = {};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (size_t i = 0; i < (size_t)framesInFlight; i++)
			{
				VK_VERIFY(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]));
			}
		}
	}

    ////////////////////////////////////////////////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////////////////////////////////////////////////
    uint32_t VulkanSwapChain::AcquireNextImage()
	{
		LU_PROFILE("VkSwapChain::AcquireImage");
		uint32_t imageIndex = 0;

		VkResult result = vkAcquireNextImageKHR(VulkanContext::GetVulkanDevice().GetVkDevice(), m_SwapChain, std::numeric_limits<uint64_t>::max(), m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
            Resize(m_Window->GetSize().x, m_Window->GetSize().y, m_Window->GetRenderer().GetSpecification().VSync, static_cast<uint8_t>(m_Window->GetRenderer().GetSpecification().Buffers));
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			LU_LOG_ERROR("[VulkanSwapChain] Failed to acquire SwapChain image!");
		}

		m_AcquiredImage = imageIndex;
		return imageIndex;
	}

	void VulkanSwapChain::FindImageFormatAndColorSpace()
    {
        const VkPhysicalDevice physicalDevice = VulkanContext::GetVulkanPhysicalDevice().GetVkPhysicalDevice();
		const VkSurfaceKHR surface = m_Surface;

		// Get list of supported surface formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());

		// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
		// there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
		if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
		{
			m_ColourFormat = VK_FORMAT_B8G8R8A8_UNORM;
			m_ColourSpace = surfaceFormats[0].colorSpace;
		}
		else
		{
			bool found_B8G8R8A8_UNORM = false;
			for (auto&& surfaceFormat : surfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
				{
					m_ColourFormat = surfaceFormat.format;
					m_ColourSpace = surfaceFormat.colorSpace;
					found_B8G8R8A8_UNORM = true;
					break;
				}
			}

			// in case VK_FORMAT_B8G8R8A8_UNORM is not available
			// select the first available color format
			if (!found_B8G8R8A8_UNORM)
			{
				m_ColourFormat = surfaceFormats[0].format;
				m_ColourSpace = surfaceFormats[0].colorSpace;
			}
		}
	}

}