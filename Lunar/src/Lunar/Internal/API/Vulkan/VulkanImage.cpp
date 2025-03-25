#include "lupch.h"
#include "VulkanImage.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include "Lunar/Internal/Enum/Fuse.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanRenderer.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanAllocator.hpp"

namespace Lunar::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// Static methods
	////////////////////////////////////////////////////////////////////////////////////
	static VkImageAspectFlags GetVulkanImageAspectFromImageUsage(VkImageUsageFlags usage);

	////////////////////////////////////////////////////////////////////////////////////
	// Init & Destroy
	////////////////////////////////////////////////////////////////////////////////////
	void VulkanImage::Init(const RendererID renderer, const ImageSpecification& specs, const SamplerSpecification& samplerSpecs)
	{
		m_RendererID = renderer;
		m_ImageSpecification = specs;
		m_SamplerSpecification = samplerSpecs;

		LU_ASSERT(((m_ImageSpecification.Usage & ImageUsage::Colour) || (m_ImageSpecification.Usage & ImageUsage::DepthStencil)), "[VulkanImage] Tried to create image without specifying if it's a Colour or Depth image.");

		CreateImage(m_ImageSpecification.Width, m_ImageSpecification.Height);
	}

	void VulkanImage::Init(const RendererID renderer, const ImageSpecification& specs, const VkImage image, const VkImageView imageView) // Note: This exists for swapchain images
	{
		m_RendererID = renderer;
		m_ImageSpecification = specs;
		m_SamplerSpecification = {};
		m_Image = image;
		m_ImageView = imageView;
	}

	void VulkanImage::Destroy()
	{
		DestroyImage();
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void VulkanImage::SetData(void* data, size_t size)
	{
		ImageLayout desiredLayout = m_ImageSpecification.Layout;

		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VmaAllocation stagingBufferAllocation = VulkanAllocator::AllocateBuffer(m_RendererID, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

		VulkanAllocator::SetData(stagingBufferAllocation, data, size);

		Transition(ImageLayout::Undefined, ImageLayout::TransferDst);
		VulkanAllocator::CopyBufferToImage(m_RendererID, stagingBuffer, m_Image, m_ImageSpecification.Width, m_ImageSpecification.Height);

		if (m_ImageSpecification.MipMaps)
		{
			GenerateMipmaps(m_Image, (VkFormat)m_ImageSpecification.Format, m_ImageSpecification.Width, m_ImageSpecification.Height, m_Miplevels);
			Transition(ImageLayout::ShaderRead, desiredLayout);
		}
		else
		{
			Transition(ImageLayout::TransferDst, desiredLayout);
		}

		VulkanAllocator::DestroyBuffer(m_RendererID, stagingBuffer, stagingBufferAllocation);
	}

	void VulkanImage::Resize(uint32_t width, uint32_t height)
	{
		Destroy();
		CreateImage(width, height);
	}

	void VulkanImage::Transition(ImageLayout initial, ImageLayout final)
	{
		if (initial == final)
			return;

		VulkanCommand command(m_RendererID, true);

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = ImageLayoutToVkImageLayout(initial);
		barrier.newLayout = ImageLayoutToVkImageLayout(final);
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.image = m_Image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = m_Miplevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage = {};
		VkPipelineStageFlags destinationStage = {};

		// Aspect checks
		if (ImageLayoutToVkImageLayout(final) == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || ImageLayoutToVkImageLayout(initial) == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			|| ImageLayoutToVkImageLayout(final) == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL || ImageLayoutToVkImageLayout(initial) == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			// Check if it has stencil component
			if (ImageFormatToVkFormat(m_ImageSpecification.Format) == VK_FORMAT_D32_SFLOAT_S8_UINT || ImageFormatToVkFormat(m_ImageSpecification.Format) == VK_FORMAT_D24_UNORM_S8_UINT)
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		switch (Enum::Fuse(ImageLayoutToVkImageLayout(initial), ImageLayoutToVkImageLayout(final)))
		{
		// Undefined
		case Enum::Fuse(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL):
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			break;
		}
		case Enum::Fuse(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL):
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		}
		case Enum::Fuse(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL):
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			break;
		}
		case Enum::Fuse(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL):
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			break;
		}
		case Enum::Fuse(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL):
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		}
		case Enum::Fuse(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL):
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		}
		case Enum::Fuse(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL):
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		}
		case Enum::Fuse(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR):
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		}

		// Colour attachment
		case Enum::Fuse(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR):
		{
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		}

		// Depth Stencil Attachments
		case Enum::Fuse(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL):
		{
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			break;
		}

		// Depth Stencil Read
		case Enum::Fuse(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL):
		{
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;
		}

		// Transfer Src
		case Enum::Fuse(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL):
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		}

		// Transfer Dst
		case Enum::Fuse(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL):
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		}

		// Shader Read
		case Enum::Fuse(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL):
		{
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		}
		case Enum::Fuse(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL):
		{
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		}
		case Enum::Fuse(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR):
		{
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = 0;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			break;
		}

		// Present Src
		case Enum::Fuse(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL):
		{
			barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		}
		case Enum::Fuse(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL):
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		}


		default:
			LU_ASSERT(false, "[VulkanImage] Layout transition not supported.");
			break;
		}

		vkCmdPipelineBarrier(command.GetVkCommandBuffer(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		command.EndAndSubmit();

		// Set the layout
		m_ImageSpecification.Layout = final;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Private methods
	////////////////////////////////////////////////////////////////////////////////////
	void VulkanImage::CreateImage(uint32_t width, uint32_t height)
	{
		ImageLayout desiredLayout = m_ImageSpecification.Layout;

		m_ImageSpecification.Width = width;
		m_ImageSpecification.Height = height;
		if (m_ImageSpecification.MipMaps)
			m_Miplevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

		m_Allocation = VulkanAllocator::AllocateImage(m_RendererID, width, height, m_Miplevels, ImageFormatToVkFormat(m_ImageSpecification.Format), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | ImageUsageToVkImageUsage(m_ImageSpecification.Usage), VMA_MEMORY_USAGE_GPU_ONLY, m_Image);

		m_ImageView = VulkanAllocator::CreateImageView(m_RendererID, m_Image, ImageFormatToVkFormat(m_ImageSpecification.Format), GetVulkanImageAspectFromImageUsage(ImageUsageToVkImageUsage(m_ImageSpecification.Usage)), m_Miplevels);
		m_Sampler = VulkanAllocator::CreateSampler(m_RendererID, FilterModeToVkFilter(m_SamplerSpecification.MagFilter), FilterModeToVkFilter(m_SamplerSpecification.MinFilter), AddressModeToVkSamplerAddressMode(m_SamplerSpecification.Address), MipmapModeToVkSamplerMipmapMode(m_SamplerSpecification.Mipmaps), m_Miplevels);

		Transition(m_ImageSpecification.Layout, desiredLayout);
	}

	void VulkanImage::GenerateMipmaps(VkImage& image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
	{
		// Check if there a no mipmaps
		if (mipLevels == 1 || mipLevels == 0)
		{
			// We transition, since we expect the the ImageLayout to ShaderRead at the end of this call.
			Transition(m_ImageSpecification.Layout, ImageLayout::ShaderRead);
			return;
		}

		// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(VulkanContext::GetVulkanPhysicalDevice().GetVkPhysicalDevice(), imageFormat, &formatProperties);

		LU_VERIFY((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT), "[VulkanImage] Texture image format does not support linear blitting!");

		VulkanCommand command = VulkanCommand(m_RendererID, true);

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(command.GetVkCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			VkImageBlit blit = {};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(command.GetVkCommandBuffer(), image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(command.GetVkCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(command.GetVkCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		command.EndAndSubmit();

		// Generating the mipmaps sets the image layout to 
		// ShaderRead, but ofcourse doesn't automatically set the 
		// specification layout. So we do it here manually.
		m_ImageSpecification.Layout = ImageLayout::ShaderRead;
	}

	void VulkanImage::DestroyImage()
	{
		VulkanRenderer::GetRenderer(m_RendererID)
			.Free([renderer = m_RendererID, sampler = m_Sampler, imageView = m_ImageView, image = m_Image, allocation = m_Allocation]()
			{
				auto device = VulkanContext::GetVulkanDevice().GetVkDevice();

				if (sampler)
					vkDestroySampler(device, sampler, nullptr);
				if (imageView)
					vkDestroyImageView(device, imageView, nullptr);

				if (image != VK_NULL_HANDLE && allocation != VK_NULL_HANDLE)
					VulkanAllocator::DestroyImage(renderer, image, allocation);
			});
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Static methods
	////////////////////////////////////////////////////////////////////////////////////
	static VkImageAspectFlags GetVulkanImageAspectFromImageUsage(VkImageUsageFlags usage)
	{
		VkImageAspectFlags flags = 0;

		if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			flags = flags | VK_IMAGE_ASPECT_COLOR_BIT;
		if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			flags = flags | VK_IMAGE_ASPECT_DEPTH_BIT;

		return flags;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Convert functions
	////////////////////////////////////////////////////////////////////////////////////
	ImageUsage VkImageUsageToImageUsage(VkImageUsageFlags usage)
	{
		ImageUsage result = ImageUsage::None;

		if (usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)							result |= ImageUsage::TransferSrc;
		if (usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)							result |= ImageUsage::TransferDst;
		if (usage & VK_IMAGE_USAGE_SAMPLED_BIT)									result |= ImageUsage::Sampled;
		if (usage & VK_IMAGE_USAGE_STORAGE_BIT)									result |= ImageUsage::Storage;
		if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)						result |= ImageUsage::Colour;
		if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)				result |= ImageUsage::DepthStencil;
		if (usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)					result |= ImageUsage::Transient;
		if (usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)						result |= ImageUsage::Input;
		if (usage & VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR)					result |= ImageUsage::VideoDecodeDstKHR;
		if (usage & VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR)					result |= ImageUsage::VideoDecodeSrcKHR;
		if (usage & VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR)					result |= ImageUsage::VideoDecodeDpbKHR;
		if (usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT)				result |= ImageUsage::FragmentDensityMapEXT;
		if (usage & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)	result |= ImageUsage::FragmentShadingRateKHR;
		if (usage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT)						result |= ImageUsage::HostTransferEXT;
		if (usage & VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR)					result |= ImageUsage::VideoEncodeDstKHR;
		if (usage & VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR)					result |= ImageUsage::VideoEncodeSrcKHR;
		if (usage & VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR)					result |= ImageUsage::VideoEncodeDpbKHR;
		if (usage & VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT)			result |= ImageUsage::FeedbackLoopEXT;
		if (usage & VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI)					result |= ImageUsage::InvocationMaskHuawei;
		if (usage & VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM)						result |= ImageUsage::SampleWeightQCOM;
		if (usage & VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM)					result |= ImageUsage::SampleBlockMatchQCOM;
		if (usage & VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV)					result |= ImageUsage::ShadingRateImageNV;
		
		return result;
	}

	VkImageUsageFlags ImageUsageToVkImageUsage(ImageUsage usage)
	{
		VkImageUsageFlags result = 0;

		if (usage & ImageUsage::TransferSrc)									result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (usage & ImageUsage::TransferDst)									result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if (usage & ImageUsage::Sampled)										result |= VK_IMAGE_USAGE_SAMPLED_BIT;
		if (usage & ImageUsage::Storage)										result |= VK_IMAGE_USAGE_STORAGE_BIT;
		if (usage & ImageUsage::Colour)											result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (usage & ImageUsage::DepthStencil)									result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		if (usage & ImageUsage::Transient)										result |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		if (usage & ImageUsage::Input)											result |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		if (usage & ImageUsage::VideoDecodeDstKHR)								result |= VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;
		if (usage & ImageUsage::VideoDecodeSrcKHR)								result |= VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR;
		if (usage & ImageUsage::VideoDecodeDpbKHR)								result |= VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR;
		if (usage & ImageUsage::FragmentDensityMapEXT)							result |= VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
		if (usage & ImageUsage::FragmentShadingRateKHR)							result |= VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
		if (usage & ImageUsage::HostTransferEXT)								result |= VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT;
		if (usage & ImageUsage::VideoEncodeDstKHR)								result |= VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR;
		if (usage & ImageUsage::VideoEncodeSrcKHR)								result |= VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
		if (usage & ImageUsage::VideoEncodeDpbKHR)								result |= VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR;
		if (usage & ImageUsage::FeedbackLoopEXT)								result |= VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT;
		if (usage & ImageUsage::InvocationMaskHuawei)							result |= VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI;
		if (usage & ImageUsage::SampleWeightQCOM)								result |= VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM;
		if (usage & ImageUsage::SampleBlockMatchQCOM)							result |= VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM;
		if (usage & ImageUsage::ShadingRateImageNV)								result |= VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV;

		return result;
	}

	ImageLayout VkImageLayoutToImageLayout(VkImageLayout layout)
	{
		switch (layout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:											return ImageLayout::Undefined;
		case VK_IMAGE_LAYOUT_GENERAL:											return ImageLayout::General;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:							return ImageLayout::Colour;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:					return ImageLayout::DepthStencil;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:					return ImageLayout::DepthStencilRead;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:							return ImageLayout::ShaderRead;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:								return ImageLayout::TransferSrc;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:								return ImageLayout::TransferDst;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:									return ImageLayout::PreInitialized;
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:		return ImageLayout::DepthReadStencil;
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:		return ImageLayout::DepthReadStencilRead;
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:							return ImageLayout::Depth;
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:							return ImageLayout::DepthRead;
		case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:						return ImageLayout::Stencil;
		case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:							return ImageLayout::StencilRead;
		case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:									return ImageLayout::Read;
		case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:								return ImageLayout::Attachment;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:									return ImageLayout::PresentSrcKHR;
		case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:								return ImageLayout::VideoDecodeDstKHR;
		case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:								return ImageLayout::VideoDecodeSrcKHR;
		case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:								return ImageLayout::VideoDecodeDpbKHR;
		case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:								return ImageLayout::SharedPresentKHR;
		case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:					return ImageLayout::FragmentDensityMapEXT;
		case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:		return ImageLayout::FragmentShadingRateKHR;
		case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR:							return ImageLayout::RenderingLocalReadKHR;
		case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:								return ImageLayout::VideoEncodeDstKHR;
		case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:								return ImageLayout::VideoEncodeSrcKHR;
		case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:								return ImageLayout::VideoEncodeDpbKHR;
		case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:				return ImageLayout::FeedbackLoopEXT;
		
		default:
			LU_ASSERT(false, "[VulkanImage] Image layout not implemented.");
			break;
		}
		return ImageLayout::Undefined;
	}

	VkImageLayout ImageLayoutToVkImageLayout(ImageLayout layout)
	{
		switch (layout)
		{
		case ImageLayout::Undefined:											return VK_IMAGE_LAYOUT_UNDEFINED;
		case ImageLayout::General:												return VK_IMAGE_LAYOUT_GENERAL;
		case ImageLayout::Colour:												return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case ImageLayout::DepthStencil:											return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		case ImageLayout::DepthStencilRead:										return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		case ImageLayout::ShaderRead:											return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case ImageLayout::TransferSrc:											return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		case ImageLayout::TransferDst:											return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		case ImageLayout::PreInitialized:										return VK_IMAGE_LAYOUT_PREINITIALIZED;
		case ImageLayout::DepthReadStencil:										return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
		case ImageLayout::DepthReadStencilRead:									return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
		case ImageLayout::Depth:												return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		case ImageLayout::DepthRead:											return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
		case ImageLayout::Stencil:												return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
		case ImageLayout::StencilRead:											return VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
		case ImageLayout::Read:													return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		case ImageLayout::Attachment:											return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
		case ImageLayout::PresentSrcKHR:										return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		case ImageLayout::VideoDecodeDstKHR:									return VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR;
		case ImageLayout::VideoDecodeSrcKHR:									return VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR;
		case ImageLayout::VideoDecodeDpbKHR:									return VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR;
		case ImageLayout::SharedPresentKHR:										return VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR;
		case ImageLayout::FragmentDensityMapEXT:								return VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
		case ImageLayout::FragmentShadingRateKHR:								return VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
		case ImageLayout::RenderingLocalReadKHR:								return VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ_KHR;
		case ImageLayout::VideoEncodeDstKHR:									return VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR;
		case ImageLayout::VideoEncodeSrcKHR:									return VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR;
		case ImageLayout::VideoEncodeDpbKHR:									return VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR;
		case ImageLayout::FeedbackLoopEXT:										return VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT;
		
		default:
			LU_ASSERT(false, "[VulkanImage] Image layout not implemented.");
			break;
		}

		return VK_IMAGE_LAYOUT_UNDEFINED;
	}

	ImageFormat VkFormatToImageFormat(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_UNDEFINED:												return ImageFormat::Undefined;
		case VK_FORMAT_R8G8B8A8_UNORM:											return ImageFormat::RGBA;
		case VK_FORMAT_B8G8R8A8_UNORM:											return ImageFormat::BGRA;
		case VK_FORMAT_R8G8B8A8_SRGB:											return ImageFormat::sRGB;
		case VK_FORMAT_D32_SFLOAT:												return ImageFormat::Depth32SFloat;
		case VK_FORMAT_D32_SFLOAT_S8_UINT:										return ImageFormat::Depth32SFloatS8;
		case VK_FORMAT_D24_UNORM_S8_UINT:										return ImageFormat::Depth24UnormS8;

		default:
			LU_ASSERT(false, "[VulkanImage] Format not implemented.");
			break;
		}

		return ImageFormat::Undefined;
	}

	VkFormat ImageFormatToVkFormat(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::Undefined:											return VK_FORMAT_UNDEFINED;
		case ImageFormat::RGBA:													return VK_FORMAT_R8G8B8A8_UNORM;
		case ImageFormat::BGRA:													return VK_FORMAT_B8G8R8A8_UNORM;
		case ImageFormat::sRGB:													return VK_FORMAT_R8G8B8A8_SRGB;
		case ImageFormat::Depth32SFloat:										return VK_FORMAT_D32_SFLOAT;
		case ImageFormat::Depth32SFloatS8:										return VK_FORMAT_D32_SFLOAT_S8_UINT;
		case ImageFormat::Depth24UnormS8:										return VK_FORMAT_D24_UNORM_S8_UINT;

		default:
			LU_ASSERT(false, "[VulkanImage] Format not implemented.");
			break;
		}

		return VK_FORMAT_UNDEFINED;
	}

	FilterMode VkFilterToFilterMode(VkFilter filter)
	{
		switch (filter)
		{
		case VK_FILTER_NEAREST:													return FilterMode::Nearest;
		case VK_FILTER_LINEAR:													return FilterMode::Linear;
		case VK_FILTER_CUBIC_EXT:												return FilterMode::CubicEXT;

		default:
			LU_ASSERT(false, "[VulkanImage] Filter not implemented.");
			break;
		}

		return FilterMode::CubicEXT;
	}

	VkFilter FilterModeToVkFilter(FilterMode filter)
	{
		switch (filter)
		{
		case FilterMode::Nearest:												return VK_FILTER_NEAREST;
		case FilterMode::Linear:												return VK_FILTER_LINEAR;
		case FilterMode::CubicEXT:												return VK_FILTER_CUBIC_EXT;

		default:
			LU_ASSERT(false, "[VulkanImage] Filter not implemented.");
			break;
		}

		return VK_FILTER_NEAREST;
	}

	AddressMode VkSamplerAddressModeToAddressMode(VkSamplerAddressMode mode)
	{
		switch (mode)
		{
		case VK_SAMPLER_ADDRESS_MODE_REPEAT:									return AddressMode::Repeat;
		case VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT:							return AddressMode::MirroredRepeat;
		case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:								return AddressMode::ClampToEdge;
		case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:							return AddressMode::ClampToBorder;
		case VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE:						return AddressMode::MirrorClampToEdge;

		default:
			LU_ASSERT(false, "[VulkanImage] Address not implemented.");
			break;
		}

		return AddressMode::Repeat;
	}

	VkSamplerAddressMode AddressModeToVkSamplerAddressMode(AddressMode mode)
	{
		switch (mode)
		{
		case AddressMode::Repeat:												return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case AddressMode::MirroredRepeat:										return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case AddressMode::ClampToEdge:											return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case AddressMode::ClampToBorder:										return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case AddressMode::MirrorClampToEdge:									return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;

		default:
			LU_ASSERT(false, "[VulkanImage] Address not implemented.");
			break;
		}

		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}

	MipmapMode VkSamplerMipmapModeToMipmapMode(VkSamplerMipmapMode mode)
	{
		switch (mode)
		{
		case VK_SAMPLER_MIPMAP_MODE_NEAREST:									return MipmapMode::Nearest;
		case VK_SAMPLER_MIPMAP_MODE_LINEAR:										return MipmapMode::Linear;

		default:
			LU_ASSERT(false, "[VulkanImage] MipmapMode not implemented.");
			break;
		}

		return MipmapMode::Nearest;
	}

	VkSamplerMipmapMode MipmapModeToVkSamplerMipmapMode(MipmapMode mode)
	{
		switch (mode)
		{
		case MipmapMode::Nearest:												return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case MipmapMode::Linear:												return VK_SAMPLER_MIPMAP_MODE_LINEAR;

		default:
			LU_ASSERT(false, "[VulkanImage] MipmapMode not implemented.");
			break;
		}

		return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	}

}