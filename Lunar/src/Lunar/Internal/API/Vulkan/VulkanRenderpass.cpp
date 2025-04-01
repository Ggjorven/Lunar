#include "lupch.h"
#include "VulkanRenderpass.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include "Lunar/Internal/Renderer/Renderer.hpp"

#include "Lunar/Internal/Renderer/Image.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanRenderer.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanImage.hpp"

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanShaderCompiler
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanRenderpass::Init(const RendererID renderer, const RenderpassSpecification& specs, CommandBuffer* commandBuffer)
    {
		m_RendererID = renderer;
		m_Specification = specs;
		m_CommandBuffer = commandBuffer;

        LU_ASSERT(((!m_Specification.ColourAttachment.empty()) || m_Specification.DepthAttachment), "No Colour or Depth image passed in.");

        Vec2<uint32_t> size = GetSize();

        CreateRenderpass();
        CreateFramebuffers(size.x, size.y);
    }

    void VulkanRenderpass::Destroy()
    {
        DestroyRenderpass();
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanRenderpass::Resize(uint32_t width, uint32_t height)
    {
        Renderer::GetRenderer(m_RendererID).Free([frameBuffers = m_Framebuffers]()
        {
            auto device = VulkanContext::GetVulkanDevice().GetVkDevice();

            for (auto& framebuffer : frameBuffers)
                vkDestroyFramebuffer(device, framebuffer, nullptr);
        });

        CreateFramebuffers(width, height);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Getters
    ////////////////////////////////////////////////////////////////////////////////////
    Vec2<uint32_t> VulkanRenderpass::GetSize() const
    {
        Vec2<uint32_t> size = {};

        if (!m_Specification.ColourAttachment.empty())
        {
            size.x = m_Specification.ColourAttachment[0]->GetSpecification().Width;
            size.y = m_Specification.ColourAttachment[0]->GetSpecification().Height;
        }
        else if (m_Specification.DepthAttachment)
        {
            size.x = m_Specification.DepthAttachment->GetSpecification().Width;
            size.y = m_Specification.DepthAttachment->GetSpecification().Height;
        }

        return size;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanRenderpass::CreateRenderpass()
    {
        std::vector<VkAttachmentDescription> attachments = { };
        std::vector<VkAttachmentReference> attachmentRefs = { };

        if (!m_Specification.ColourAttachment.empty())
        {
            VkAttachmentDescription& colorAttachment = attachments.emplace_back();
            colorAttachment.format = ImageFormatToVkFormat(m_Specification.ColourAttachment[0]->GetSpecification().Format);
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = LoadOperationToVkAttachmentLoadOp(m_Specification.ColourLoadOp);
            colorAttachment.storeOp = StoreOperationToVkAttachmentStoreOp(m_Specification.ColourStoreOp);
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = ImageLayoutToVkImageLayout(m_Specification.PreviousColourImageLayout);
            colorAttachment.finalLayout = ImageLayoutToVkImageLayout(m_Specification.FinalColourImageLayout);

            VkAttachmentReference& colorAttachmentRef = attachmentRefs.emplace_back();
            colorAttachmentRef.attachment = static_cast<uint32_t>((attachments.size() - 1));
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        if (m_Specification.DepthAttachment)
        {
            VkAttachmentDescription& depthAttachment = attachments.emplace_back();
            depthAttachment.format = ImageFormatToVkFormat(m_Specification.DepthAttachment->GetSpecification().Format);
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = LoadOperationToVkAttachmentLoadOp(m_Specification.DepthLoadOp);
            depthAttachment.storeOp = StoreOperationToVkAttachmentStoreOp(m_Specification.DepthStoreOp);
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = ImageLayoutToVkImageLayout(m_Specification.PreviousDepthImageLayout);
            depthAttachment.finalLayout = ImageLayoutToVkImageLayout(m_Specification.FinalDepthImageLayout);

            VkAttachmentReference& depthAttachmentRef = attachmentRefs.emplace_back();
            depthAttachmentRef.attachment = static_cast<uint32_t>((attachments.size() - 1));
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        if (!m_Specification.ColourAttachment.empty())
        {
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &attachmentRefs[0];
        }
        else
        {
            subpass.colorAttachmentCount = 0;
            subpass.pColorAttachments = nullptr;
        }

        if (m_Specification.DepthAttachment)
        {
            if (!m_Specification.ColourAttachment.empty())
                subpass.pDepthStencilAttachment = &attachmentRefs[1];
            else
                subpass.pDepthStencilAttachment = &attachmentRefs[0];
        }

        std::vector<VkSubpassDependency> dependencies = GetDependencies(m_Specification.Usage);

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        VK_VERIFY(vkCreateRenderPass(VulkanContext::GetVulkanDevice().GetVkDevice(), &renderPassInfo, nullptr, &m_RenderPass));
    }

    void VulkanRenderpass::CreateFramebuffers(uint32_t width, uint32_t height)
    {
        // Framebuffer creation
        m_Framebuffers.resize(VulkanRenderer::GetRenderer(m_RendererID).GetVulkanSwapChain().GetSwapChainImages().size());
        for (size_t i = 0; i < m_Framebuffers.size(); i++)
        {
            std::vector<VkImageView> attachments = { };
            if (!m_Specification.ColourAttachment.empty())
            {
                if (m_Specification.ColourAttachment.size() == 1)
                {
                    VulkanImage& vkImage = m_Specification.ColourAttachment[0]->GetInternalImage();
                    attachments.push_back(vkImage.GetVkImageView());
                }
                else // If the size is not equal to 1 it has to be equal to the amount of swapchain images
                {
                    VulkanImage& vkImage = m_Specification.ColourAttachment[i]->GetInternalImage();
                    attachments.push_back(vkImage.GetVkImageView());
                }
            }
            if (m_Specification.DepthAttachment)
            {
                VulkanImage& vkImage = m_Specification.DepthAttachment->GetInternalImage();
                attachments.push_back(vkImage.GetVkImageView());
            }

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_RenderPass;
            framebufferInfo.attachmentCount = (uint32_t)attachments.size();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = width;
            framebufferInfo.height = height;
            framebufferInfo.layers = 1;

            VK_VERIFY(vkCreateFramebuffer(VulkanContext::GetVulkanDevice().GetVkDevice(), &framebufferInfo, nullptr, &m_Framebuffers[i]));
        }
    }

    void VulkanRenderpass::DestroyRenderpass()
    {
        Renderer::GetRenderer(m_RendererID).Free([frameBuffers = m_Framebuffers, renderpass = m_RenderPass]()
        {
            auto device = VulkanContext::GetVulkanDevice().GetVkDevice();

            for (auto& framebuffer : frameBuffers)
                vkDestroyFramebuffer(device, framebuffer, nullptr);

            vkDestroyRenderPass(device, renderpass, nullptr);
        });
    }

    std::vector<VkSubpassDependency> VulkanRenderpass::GetDependencies(RenderpassUsage usage)
    {
        switch (usage)
        {
        case RenderpassUsage::Forward:
        {
            std::vector<VkSubpassDependency> dependencies(2);
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask = 0;
            dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = 0;
            dependencies[1].dependencyFlags = 0; // No special flags needed

            return dependencies;
        }
        case RenderpassUsage::ComputeToTexture:
        {
            std::vector<VkSubpassDependency> dependencies(1);
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            return dependencies;
        }
        case RenderpassUsage::Offscreen:
        {
            std::vector<VkSubpassDependency> dependencies(1);
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            return dependencies;
        }

        default:
            LU_ASSERT(false, "[VkRenderpass] Invalid usage passed in.");
            break;
        }

        return { };
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Convert functions
    ////////////////////////////////////////////////////////////////////////////////////
    LoadOperation VkAttachmentLoadOpToLoadOperation(VkAttachmentLoadOp loadOp)
    {
        switch (loadOp)
        {
		case VK_ATTACHMENT_LOAD_OP_NONE:            return LoadOperation::None;
		case VK_ATTACHMENT_LOAD_OP_LOAD:            return LoadOperation::Load;
		case VK_ATTACHMENT_LOAD_OP_CLEAR:           return LoadOperation::Clear;
		case VK_ATTACHMENT_LOAD_OP_DONT_CARE:       return LoadOperation::DontCare;
		
        default: 
            LU_ASSERT(false, "[VkRenderpass] Unknown LoadOperation!");
            break;
        }

        return LoadOperation::None;
    }

    VkAttachmentLoadOp LoadOperationToVkAttachmentLoadOp(LoadOperation loadOp)
    {
        switch (loadOp)
        {
        case LoadOperation::None:                   return VK_ATTACHMENT_LOAD_OP_NONE;
        case LoadOperation::Load:                   return VK_ATTACHMENT_LOAD_OP_LOAD;
        case LoadOperation::Clear:                  return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case LoadOperation::DontCare:               return VK_ATTACHMENT_LOAD_OP_DONT_CARE;

        default:
            LU_ASSERT(false, "[VkRenderpass] Unknown LoadOperation!");
            break;
        }

        return VK_ATTACHMENT_LOAD_OP_NONE;
    }

    StoreOperation VkAttachmentStoreOpToStoreOperation(VkAttachmentStoreOp storeOp)
    {
        switch (storeOp)
        {
        case VK_ATTACHMENT_STORE_OP_NONE:            return StoreOperation::None;
        case VK_ATTACHMENT_STORE_OP_STORE:           return StoreOperation::Store;
        case VK_ATTACHMENT_STORE_OP_DONT_CARE:       return StoreOperation::DontCare;

		default:
			LU_ASSERT(false, "[VkRenderpass] Unknown StoreOperation!");
			break;
        }

        return StoreOperation::None;
    }

    VkAttachmentStoreOp StoreOperationToVkAttachmentStoreOp(StoreOperation storeOp)
    {
        switch (storeOp)
        {
		case StoreOperation::None:                  return VK_ATTACHMENT_STORE_OP_NONE;
		case StoreOperation::Store:                 return VK_ATTACHMENT_STORE_OP_STORE;
		case StoreOperation::DontCare:              return VK_ATTACHMENT_STORE_OP_DONT_CARE;

        default:
            LU_ASSERT(false, "[VkRenderpass] Unknown StoreOperation!");
            break;
        }

		return VK_ATTACHMENT_STORE_OP_NONE;
    }

}
