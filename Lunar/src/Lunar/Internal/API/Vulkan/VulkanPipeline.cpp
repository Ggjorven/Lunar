#include "lupch.h"
#include "VulkanPipeline.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include "Lunar/Internal/Renderer/Image.hpp"
#include "Lunar/Internal/Renderer/Shader.hpp"
#include "Lunar/Internal/Renderer/Renderer.hpp"
#include "Lunar/Internal/Renderer/Descriptor.hpp"
#include "Lunar/Internal/Renderer/Renderpass.hpp"
#include "Lunar/Internal/Renderer/CommandBuffer.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanImage.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanDevice.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanBuffers.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanAllocator.hpp"

namespace
{

    static VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
    {
        auto func = (PFN_vkCreateRayTracingPipelinesKHR)vkGetInstanceProcAddr(Lunar::Internal::VulkanContext::GetVkInstance(), "vkCreateRayTracingPipelinesKHR");

        if (func != nullptr)
            return func(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    static VKAPI_ATTR VkResult VKAPI_CALL CreateRayTracingPipelinesNV(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoNV* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
    {
        auto func = (PFN_vkCreateRayTracingPipelinesNV)vkGetInstanceProcAddr(Lunar::Internal::VulkanContext::GetVkInstance(), "vkCreateRayTracingPipelinesNV");

        if (func != nullptr)
            return func(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);

        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

}

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Init & Destroy
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanPipeline::Init(const RendererID renderer, const PipelineSpecification& specs, DescriptorSets& sets, Shader& shader)
    {
        m_RendererID = renderer;
        m_Specification = specs;

        switch (specs.Usage)
        {
        case PipelineUsage::Graphics:
            CreateGraphicsPipeline(sets, shader, nullptr);
            break;
        case PipelineUsage::Compute:
            CreateComputePipeline(sets, shader);
            break;
        case PipelineUsage::RayTracingKHR:
            CreateRayTracingPipelineKHR(sets, shader);
            break;
        case PipelineUsage::RayTracingNV:
            CreateRayTracingPipelineNV(sets, shader);
            break;

        default:
            LU_ASSERT(false, "[VkPipeline] Invalid PipelineType selected.");
            break;
        }
    }

    void VulkanPipeline::Init(const RendererID renderer, const PipelineSpecification& specs, DescriptorSets& sets, Shader& shader, Renderpass& renderpass)
    {
        m_RendererID = renderer;
        m_Specification = specs;

        LU_ASSERT((specs.Usage == PipelineUsage::Graphics), "[VkPipeline] Used pipeline graphics constructor but Type != PipelineType::Graphics");
        CreateGraphicsPipeline(sets, shader, &renderpass);
    }

    void VulkanPipeline::Destroy()
    {
        Renderer::GetRenderer(m_RendererID).Free([pipeline = m_Pipeline, pipelineLayout = m_PipelineLayout]()
        {
            auto device = VulkanContext::GetVulkanDevice().GetVkDevice();

            vkDestroyPipeline(device, pipeline, nullptr);
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        });
    }

    void VulkanPipeline::Use(CommandBuffer& cmdBuf, PipelineBindPoint bindPoint)
    {
        VulkanCommandBuffer& vkCmdBuf = cmdBuf.GetInternalCommandBuffer();

        vkCmdBindPipeline(vkCmdBuf.GetVkCommandBuffer(VulkanRenderer::GetRenderer(m_RendererID).GetVulkanSwapChain().GetCurrentFrame()), PipelineBindPointToVkPipelineBindPoint(bindPoint), m_Pipeline);
    }

    void VulkanPipeline::PushConstant(CommandBuffer& cmdBuf, ShaderStage stage, void* data)
    {
        PushConstant(cmdBuf, stage, data, m_Specification.PushConstants[stage].Offset, m_Specification.PushConstants[stage].Size);
    }

    void VulkanPipeline::PushConstant(CommandBuffer& cmdBuf, ShaderStage stage, void* data, size_t offset, size_t size)
    {
        LU_ASSERT((!m_Specification.PushConstants.empty()), "[VkPipeline] No push constant range(s) defined on pipeline creation.");

        VulkanCommandBuffer& vkCmdBuf = cmdBuf.GetInternalCommandBuffer();
        vkCmdPushConstants(vkCmdBuf.GetVkCommandBuffer(VulkanRenderer::GetRenderer(m_RendererID).GetVulkanSwapChain().GetCurrentFrame()), m_PipelineLayout, ShaderStageToVkShaderStageFlags(stage), static_cast<uint32_t>(offset), static_cast<uint32_t>(size), data);
    }

    void VulkanPipeline::DispatchCompute(CommandBuffer& cmdBuf, uint32_t width, uint32_t height, uint32_t depth)
    {
        VulkanCommandBuffer& vkCmdBuf = cmdBuf.GetInternalCommandBuffer();

        vkCmdDispatch(vkCmdBuf.GetVkCommandBuffer(VulkanRenderer::GetRenderer(m_RendererID).GetVulkanSwapChain().GetCurrentFrame()), width, height, depth);
    }

    void VulkanPipeline::CreateGraphicsPipeline(DescriptorSets& sets, Shader& shader, Renderpass* renderpass) // Renderpass may be nullptr
    {
        VulkanShader& vkShader = shader.GetInternalShader();

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { };
        if (vkShader.GetShaders().contains(ShaderStage::Vertex))
        {
            VkPipelineShaderStageCreateInfo& vertShaderStageInfo = shaderStages.emplace_back();
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = vkShader.GetShader(ShaderStage::Vertex);
            vertShaderStageInfo.pName = "main";
        }

        if (vkShader.GetShaders().contains(ShaderStage::Fragment))
        {
            VkPipelineShaderStageCreateInfo& fragShaderStageInfo = shaderStages.emplace_back();
            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = vkShader.GetShader(ShaderStage::Fragment);
            fragShaderStageInfo.pName = "main";
        }

        auto bindingDescriptions = GetBindingDescriptions();
        auto attributeDescriptions = GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        if (!m_Specification.Bufferlayout.GetElements().empty())
        {
            vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        }
        else
        {
            vertexInputInfo.vertexBindingDescriptionCount = 0;
            vertexInputInfo.vertexAttributeDescriptionCount = 0;
            vertexInputInfo.pVertexAttributeDescriptions = nullptr;
            vertexInputInfo.pVertexBindingDescriptions = nullptr;
        }

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = PolygonModeToVkPolygonMode(m_Specification.Polygonmode);
        rasterizer.lineWidth = m_Specification.LineWidth;
        rasterizer.cullMode = CullingModeToVkCullModeFlags(m_Specification.Cullingmode);
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = m_Specification.Blending;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 1.0f;
        colorBlending.blendConstants[1] = 1.0f;
        colorBlending.blendConstants[2] = 1.0f;
        colorBlending.blendConstants[3] = 1.0f;

        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // Descriptor layouts
        VulkanDescriptorSets& vkDescriptorSets = sets.GetInternalDescriptorSets();

        std::vector<VkDescriptorSetLayout> descriptorLayouts = { };
        descriptorLayouts.reserve(vkDescriptorSets.m_DescriptorLayouts.size());

        for (auto& layout : vkDescriptorSets.m_DescriptorLayouts)
            descriptorLayouts.push_back(layout);

        // Push constants
        std::vector<VkPushConstantRange> pushConstants;
        pushConstants.reserve(m_Specification.PushConstants.size());

        for (auto& [stage, info] : m_Specification.PushConstants)
        {
            LU_ASSERT((info.Size > 0), "[VkPipeline] Push constant range passed has size of 0.");

            VkPushConstantRange& range = pushConstants.emplace_back();
            range.stageFlags = ShaderStageToVkShaderStageFlags(stage);
            range.offset = static_cast<uint32_t>(info.Offset);
            range.size = static_cast<uint32_t>(info.Size);
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();

        VK_VERIFY(vkCreatePipelineLayout(VulkanContext::GetVulkanDevice().GetVkDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

        VkFormat dynamicColourFormat = ImageFormatToVkFormat(m_Specification.DynamicColourFormat);

        VkPipelineRenderingCreateInfo dynamicRenderInfo = {};
        dynamicRenderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
        dynamicRenderInfo.colorAttachmentCount = 1; // Note: We only support 1 attachment at the moment for dynamic rendering
        dynamicRenderInfo.pColorAttachmentFormats = &dynamicColourFormat;
        dynamicRenderInfo.depthAttachmentFormat = ImageFormatToVkFormat(m_Specification.DynamicDepthFormat);
        dynamicRenderInfo.stencilAttachmentFormat = ImageFormatToVkFormat(m_Specification.DynamicStencilFormat);

        // Create the actual graphics pipeline (where we actually use the shaders and other info)
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = (renderpass == nullptr ? &dynamicRenderInfo : nullptr);
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.renderPass = (renderpass ? renderpass->GetInternalRenderpass().GetVkRenderPass() : nullptr);
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        VK_VERIFY(vkCreateGraphicsPipelines(VulkanContext::GetVulkanDevice().GetVkDevice(), VulkanAllocator::s_PipelineCache, 1, &pipelineInfo, nullptr, &m_Pipeline));
    }

    void VulkanPipeline::CreateComputePipeline(DescriptorSets& sets, Shader& shader)
    {
        VulkanShader& vkShader = shader.GetInternalShader();

        VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
        computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        computeShaderStageInfo.module = vkShader.GetShader(ShaderStage::Compute);
        computeShaderStageInfo.pName = "main";

        // Descriptor layouts
        VulkanDescriptorSets& vkDescriptorSets = sets.GetInternalDescriptorSets();

        std::vector<VkDescriptorSetLayout> descriptorLayouts;
        descriptorLayouts.reserve(vkDescriptorSets.m_DescriptorLayouts.size());

        for (auto& layout : vkDescriptorSets.m_DescriptorLayouts)
            descriptorLayouts.push_back(layout);

        // Push constants
        std::vector<VkPushConstantRange> pushConstants;
        pushConstants.reserve(m_Specification.PushConstants.size());

        for (auto& [stage, info] : m_Specification.PushConstants)
        {
            LU_ASSERT((info.Size > 0), "[VkPipeline] Push constant range passed has size of 0.");

            VkPushConstantRange& range = pushConstants.emplace_back();
            range.stageFlags = ShaderStageToVkShaderStageFlags(stage);
            range.offset = static_cast<uint32_t>(info.Offset);
            range.size = static_cast<uint32_t>(info.Size);
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();

        VK_VERIFY(vkCreatePipelineLayout(VulkanContext::GetVulkanDevice().GetVkDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

        VkComputePipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.stage = computeShaderStageInfo;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        VK_VERIFY(vkCreateComputePipelines(VulkanContext::GetVulkanDevice().GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline));
    }

    void VulkanPipeline::CreateRayTracingPipelineKHR(DescriptorSets& sets, Shader& shader)
    {
        VulkanShader& vkShader = shader.GetInternalShader();

        std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups = { };
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { };
        std::unordered_map<ShaderStage, uint32_t> shaderStageIndices = { };

        // Shader Stages
        uint32_t currentIndex = 0;
        if (vkShader.GetShaders().contains(ShaderStage::RayGenKHR))
        {
            VkPipelineShaderStageCreateInfo& raygenShaderStageInfo = shaderStages.emplace_back();
            raygenShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            raygenShaderStageInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
            raygenShaderStageInfo.module = vkShader.GetShader(ShaderStage::RayGenKHR);
            raygenShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::RayGenKHR] = currentIndex++;
        }

        if (vkShader.GetShaders().contains(ShaderStage::MissKHR))
        {
            VkPipelineShaderStageCreateInfo& missShaderStageInfo = shaderStages.emplace_back();
            missShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            missShaderStageInfo.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
            missShaderStageInfo.module = vkShader.GetShader(ShaderStage::MissKHR);
            missShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::MissKHR] = currentIndex++;
        }

        if (vkShader.GetShaders().contains(ShaderStage::ClosestHitKHR))
        {
            VkPipelineShaderStageCreateInfo& hitShaderStageInfo = shaderStages.emplace_back();
            hitShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            hitShaderStageInfo.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
            hitShaderStageInfo.module = vkShader.GetShader(ShaderStage::ClosestHitKHR);
            hitShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::ClosestHitKHR] = currentIndex++;
        }

        if (vkShader.GetShaders().contains(ShaderStage::AnyHitKHR))
        {
            VkPipelineShaderStageCreateInfo& anyHitShaderStageInfo = shaderStages.emplace_back();
            anyHitShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            anyHitShaderStageInfo.stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
            anyHitShaderStageInfo.module = vkShader.GetShader(ShaderStage::AnyHitKHR);
            anyHitShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::AnyHitKHR] = currentIndex++;
        }

        if (vkShader.GetShaders().contains(ShaderStage::IntersectionKHR))
        {
            VkPipelineShaderStageCreateInfo& intersectionShaderStageInfo = shaderStages.emplace_back();
            intersectionShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            intersectionShaderStageInfo.stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
            intersectionShaderStageInfo.module = vkShader.GetShader(ShaderStage::IntersectionKHR);
            intersectionShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::IntersectionKHR] = currentIndex++;
        }

        // Shader groups
        if (vkShader.GetShaders().contains(ShaderStage::RayGenKHR))
        {
            VkRayTracingShaderGroupCreateInfoKHR& raygenGroupInfo = shaderGroups.emplace_back();
            raygenGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            raygenGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            raygenGroupInfo.generalShader = shaderStageIndices[ShaderStage::RayGenKHR];
            raygenGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
            raygenGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
            raygenGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
        }

        if (vkShader.GetShaders().contains(ShaderStage::MissKHR))
        {
            VkRayTracingShaderGroupCreateInfoKHR& missGroupInfo = shaderGroups.emplace_back();
            missGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            missGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            missGroupInfo.generalShader = shaderStageIndices[ShaderStage::MissKHR];
            missGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
            missGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
            missGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
        }

        if (vkShader.GetShaders().contains(ShaderStage::ClosestHitKHR))
        {
            VkRayTracingShaderGroupCreateInfoKHR& hitGroupInfo = shaderGroups.emplace_back();
            hitGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            hitGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            hitGroupInfo.generalShader = VK_SHADER_UNUSED_KHR;
            hitGroupInfo.closestHitShader = shaderStageIndices[ShaderStage::ClosestHitKHR];
            hitGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
            hitGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
        }

        if (vkShader.GetShaders().contains(ShaderStage::AnyHitKHR))
        {
            VkRayTracingShaderGroupCreateInfoKHR& anyHitGroupInfo = shaderGroups.emplace_back();
            anyHitGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            anyHitGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            anyHitGroupInfo.generalShader = VK_SHADER_UNUSED_KHR;
            anyHitGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
            anyHitGroupInfo.anyHitShader = shaderStageIndices[ShaderStage::AnyHitKHR];
            anyHitGroupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
        }

        if (vkShader.GetShaders().contains(ShaderStage::IntersectionKHR))
        {
            VkRayTracingShaderGroupCreateInfoKHR& intersectionGroupInfo = shaderGroups.emplace_back();
            intersectionGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            intersectionGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
            intersectionGroupInfo.generalShader = VK_SHADER_UNUSED_KHR;
            intersectionGroupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
            intersectionGroupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
            intersectionGroupInfo.intersectionShader = shaderStageIndices[ShaderStage::IntersectionKHR];
        }

        // Descriptor layouts
        VulkanDescriptorSets& vkDescriptorSets = sets.GetInternalDescriptorSets();

        std::vector<VkDescriptorSetLayout> descriptorLayouts;
        descriptorLayouts.reserve(vkDescriptorSets.m_DescriptorLayouts.size());

        for (auto& layout : vkDescriptorSets.m_DescriptorLayouts)
            descriptorLayouts.push_back(layout);

        // Push constants
        std::vector<VkPushConstantRange> pushConstants;
        pushConstants.reserve(m_Specification.PushConstants.size());

        for (auto& [stage, info] : m_Specification.PushConstants)
        {
            LU_ASSERT((info.Size > 0), "[VkPipeline] Push constant range passed has size of 0.");

            VkPushConstantRange& range = pushConstants.emplace_back();
            range.stageFlags = ShaderStageToVkShaderStageFlags(stage);
            range.offset = static_cast<uint32_t>(info.Offset);
            range.size = static_cast<uint32_t>(info.Size);
        }

        // Layout create info
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();

        VK_VERIFY(vkCreatePipelineLayout(VulkanContext::GetVulkanDevice().GetVkDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

        // Pipeline create info
        VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfo = {};
        rayTracingPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        rayTracingPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        rayTracingPipelineCreateInfo.pStages = shaderStages.data();
        rayTracingPipelineCreateInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
        rayTracingPipelineCreateInfo.pGroups = shaderGroups.data();
        rayTracingPipelineCreateInfo.maxPipelineRayRecursionDepth = m_Specification.MaxRayRecursion;
        rayTracingPipelineCreateInfo.pLibraryInfo = nullptr;
        rayTracingPipelineCreateInfo.pLibraryInterface = nullptr;
        rayTracingPipelineCreateInfo.pDynamicState = nullptr;
        rayTracingPipelineCreateInfo.flags = 0; // Adjust as needed

        // Create the ray tracing pipeline
        VK_VERIFY(CreateRayTracingPipelinesKHR(VulkanContext::GetVulkanDevice().GetVkDevice(), VK_NULL_HANDLE, VulkanAllocator::s_PipelineCache, 1, &rayTracingPipelineCreateInfo, nullptr, &m_Pipeline));
    }

    void VulkanPipeline::CreateRayTracingPipelineNV(DescriptorSets& sets, Shader& shader)
    {
        VulkanShader& vkShader = shader.GetInternalShader();

        std::vector<VkRayTracingShaderGroupCreateInfoNV> shaderGroups;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        std::unordered_map<ShaderStage, uint32_t> shaderStageIndices;

        // Shader Stages
        uint32_t currentIndex = 0;
        if (vkShader.GetShaders().contains(ShaderStage::RayGenNV))
        {
            VkPipelineShaderStageCreateInfo& raygenShaderStageInfo = shaderStages.emplace_back();
            raygenShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            raygenShaderStageInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
            raygenShaderStageInfo.module = vkShader.GetShader(ShaderStage::RayGenNV);
            raygenShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::RayGenNV] = currentIndex++;
        }

        if (vkShader.GetShaders().contains(ShaderStage::MissNV))
        {
            VkPipelineShaderStageCreateInfo& missShaderStageInfo = shaderStages.emplace_back();
            missShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            missShaderStageInfo.stage = VK_SHADER_STAGE_MISS_BIT_NV;
            missShaderStageInfo.module = vkShader.GetShader(ShaderStage::MissNV);
            missShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::MissNV] = currentIndex++;
        }

        if (vkShader.GetShaders().contains(ShaderStage::ClosestHitNV))
        {
            VkPipelineShaderStageCreateInfo& hitShaderStageInfo = shaderStages.emplace_back();
            hitShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            hitShaderStageInfo.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
            hitShaderStageInfo.module = vkShader.GetShader(ShaderStage::ClosestHitNV);
            hitShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::ClosestHitNV] = currentIndex++;
        }

        if (vkShader.GetShaders().contains(ShaderStage::AnyHitNV))
        {
            VkPipelineShaderStageCreateInfo& anyHitShaderStageInfo = shaderStages.emplace_back();
            anyHitShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            anyHitShaderStageInfo.stage = VK_SHADER_STAGE_ANY_HIT_BIT_NV;
            anyHitShaderStageInfo.module = vkShader.GetShader(ShaderStage::AnyHitNV);
            anyHitShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::AnyHitKHR] = currentIndex++;
        }

        if (vkShader.GetShaders().contains(ShaderStage::IntersectionNV))
        {
            VkPipelineShaderStageCreateInfo& intersectionShaderStageInfo = shaderStages.emplace_back();
            intersectionShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            intersectionShaderStageInfo.stage = VK_SHADER_STAGE_INTERSECTION_BIT_NV;
            intersectionShaderStageInfo.module = vkShader.GetShader(ShaderStage::IntersectionNV);
            intersectionShaderStageInfo.pName = "main";

            shaderStageIndices[ShaderStage::IntersectionNV] = currentIndex++;
        }

        // Shader groups
        if (vkShader.GetShaders().contains(ShaderStage::RayGenNV))
        {
            VkRayTracingShaderGroupCreateInfoNV& raygenGroupInfo = shaderGroups.emplace_back();
            raygenGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
            raygenGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
            raygenGroupInfo.generalShader = shaderStageIndices[ShaderStage::RayGenNV];
            raygenGroupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
            raygenGroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
            raygenGroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
        }

        if (vkShader.GetShaders().contains(ShaderStage::MissNV))
        {
            VkRayTracingShaderGroupCreateInfoNV& missGroupInfo = shaderGroups.emplace_back();
            missGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
            missGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
            missGroupInfo.generalShader = shaderStageIndices[ShaderStage::MissNV];
            missGroupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
            missGroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
            missGroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
        }

        if (vkShader.GetShaders().contains(ShaderStage::ClosestHitNV))
        {
            VkRayTracingShaderGroupCreateInfoNV& hitGroupInfo = shaderGroups.emplace_back();
            hitGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
            hitGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
            hitGroupInfo.generalShader = VK_SHADER_UNUSED_NV;
            hitGroupInfo.closestHitShader = shaderStageIndices[ShaderStage::ClosestHitNV];
            hitGroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
            hitGroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
        }

        if (vkShader.GetShaders().contains(ShaderStage::AnyHitNV))
        {
            VkRayTracingShaderGroupCreateInfoNV& anyHitGroupInfo = shaderGroups.emplace_back();
            anyHitGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
            anyHitGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
            anyHitGroupInfo.generalShader = VK_SHADER_UNUSED_NV;
            anyHitGroupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
            anyHitGroupInfo.anyHitShader = shaderStageIndices[ShaderStage::AnyHitNV];
            anyHitGroupInfo.intersectionShader = VK_SHADER_UNUSED_NV;
        }

        if (vkShader.GetShaders().contains(ShaderStage::IntersectionNV))
        {
            VkRayTracingShaderGroupCreateInfoNV& intersectionGroupInfo = shaderGroups.emplace_back();
            intersectionGroupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
            intersectionGroupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_NV;
            intersectionGroupInfo.generalShader = VK_SHADER_UNUSED_NV;
            intersectionGroupInfo.closestHitShader = VK_SHADER_UNUSED_NV;
            intersectionGroupInfo.anyHitShader = VK_SHADER_UNUSED_NV;
            intersectionGroupInfo.intersectionShader = shaderStageIndices[ShaderStage::IntersectionNV];
        }

        // Descriptor layouts
        VulkanDescriptorSets& vkDescriptorSets = sets.GetInternalDescriptorSets();

        std::vector<VkDescriptorSetLayout> descriptorLayouts;
        descriptorLayouts.reserve(vkDescriptorSets.m_DescriptorLayouts.size());

        for (auto& layout : vkDescriptorSets.m_DescriptorLayouts)
            descriptorLayouts.push_back(layout);

        // Push constants
        std::vector<VkPushConstantRange> pushConstants;
        pushConstants.reserve(m_Specification.PushConstants.size());

        for (auto& [stage, info] : m_Specification.PushConstants)
        {
            LU_ASSERT((info.Size > 0), "[VkPipeline] Push constant range passed has size of 0.");

            VkPushConstantRange& range = pushConstants.emplace_back();
            range.stageFlags = ShaderStageToVkShaderStageFlags(stage);
            range.offset = static_cast<uint32_t>(info.Offset);
            range.size = static_cast<uint32_t>(info.Size);
        }

        // Layout create info
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();

        VK_VERIFY(vkCreatePipelineLayout(VulkanContext::GetVulkanDevice().GetVkDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

        // Pipeline create info
        VkRayTracingPipelineCreateInfoNV rayTracingPipelineCreateInfo = {};
        rayTracingPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
        rayTracingPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        rayTracingPipelineCreateInfo.pStages = shaderStages.data();
        rayTracingPipelineCreateInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
        rayTracingPipelineCreateInfo.pGroups = shaderGroups.data();
        rayTracingPipelineCreateInfo.maxRecursionDepth = m_Specification.MaxRayRecursion;
        rayTracingPipelineCreateInfo.flags = 0; // Adjust as needed

        // Create the ray tracing pipeline
        VK_VERIFY(CreateRayTracingPipelinesNV(VulkanContext::GetVulkanDevice().GetVkDevice(), VulkanAllocator::s_PipelineCache, 1, &rayTracingPipelineCreateInfo, nullptr, &m_Pipeline));
    }

    std::vector<VkVertexInputBindingDescription> VulkanPipeline::GetBindingDescriptions() const
    {
        std::vector<VkVertexInputBindingDescription> descriptions;
        const auto& layout = m_Specification.Bufferlayout;
        const auto& elements = layout.GetElements();

        std::unordered_map<VertexInputRate, uint32_t> rateToBinding = {};
        uint32_t bindingIndex = 0;

        for (const auto& element : elements)
        {
            if (rateToBinding.find(element.InputRate) == rateToBinding.end())
            {
                VkVertexInputBindingDescription desc = {};
                desc.binding = bindingIndex;
                desc.stride = static_cast<uint32_t>(layout.GetStride(element.InputRate));
                desc.inputRate = VertexInputRateToVkVertexInputRate(element.InputRate);

                descriptions.push_back(desc);
                rateToBinding[element.InputRate] = bindingIndex;
                bindingIndex++;
            }
        }

        return descriptions;
    }

    std::vector<VkVertexInputAttributeDescription> VulkanPipeline::GetAttributeDescriptions() const
    {
        std::vector<VkVertexInputAttributeDescription> descriptions;
        const auto& layout = m_Specification.Bufferlayout;
        const auto& elements = layout.GetElements();

        uint32_t location = 0;
        std::unordered_map<VertexInputRate, uint32_t> rateToBinding;
        uint32_t bindingIndex = 0;

        // First assign bindings to input rates
        for (const auto& element : elements)
        {
            if (rateToBinding.find(element.InputRate) == rateToBinding.end())
                rateToBinding[element.InputRate] = bindingIndex++;
        }

        uint32_t offsetVertex = 0;
        uint32_t offsetInstance = 0;

        for (const auto& element : elements)
        {
            VkVertexInputAttributeDescription attr = {};
            attr.location = location++;
            attr.binding = rateToBinding[element.InputRate];
            attr.format = DataTypeToVkFormat(element.Type);

            // Set offset based on the stride accumulation
            if (element.InputRate == VertexInputRate::Vertex)
            {
                attr.offset = offsetVertex;
                offsetVertex += static_cast<uint32_t>(DataTypeSize(element.Type));
            }
            else
            {
                attr.offset = offsetInstance;
                offsetInstance += static_cast<uint32_t>(DataTypeSize(element.Type));
            }

            descriptions.push_back(attr);
        }

        return descriptions;
    }

	////////////////////////////////////////////////////////////////////////////////////
	// Convert functions
	////////////////////////////////////////////////////////////////////////////////////
    PipelineStage VkPipelineStageToPipelineStage(VkPipelineStageFlags stage)
    {
        PipelineStage result = PipelineStage::None;

        if (stage & VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)                          result |= PipelineStage::TopOfPipe;
		if (stage & VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT)                        result |= PipelineStage::DrawIndirect;
		if (stage & VK_PIPELINE_STAGE_VERTEX_INPUT_BIT)						    result |= PipelineStage::VertexInput;
		if (stage & VK_PIPELINE_STAGE_VERTEX_SHADER_BIT)						result |= PipelineStage::VertexShader;
		if (stage & VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT)          result |= PipelineStage::TessellationControlShader;
		if (stage & VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT)	    result |= PipelineStage::TessellationEvaluationShader;
		if (stage & VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT)                      result |= PipelineStage::GeometryShader;
		if (stage & VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)					    result |= PipelineStage::FragmentShader;
		if (stage & VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT)                 result |= PipelineStage::EarlyFragmentTests;
		if (stage & VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT)                  result |= PipelineStage::LateFragmentTests;
		if (stage & VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)              result |= PipelineStage::ColourAttachmentOutput;
		if (stage & VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)					    result |= PipelineStage::ComputeShader;
		if (stage & VK_PIPELINE_STAGE_TRANSFER_BIT)                             result |= PipelineStage::Transfer;
		if (stage & VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)                       result |= PipelineStage::BottomOfPipe;
		if (stage & VK_PIPELINE_STAGE_HOST_BIT)                                 result |= PipelineStage::Host;
		if (stage & VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT)						    result |= PipelineStage::AllGraphics;
		if (stage & VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)                         result |= PipelineStage::AllCommands;
		if (stage & VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT)               result |= PipelineStage::TransformFeedbackEXT;
		if (stage & VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT)            result |= PipelineStage::ConditionalRenderingEXT;
		if (stage & VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR)     result |= PipelineStage::AccelerationStructureBuildKHR;
		if (stage & VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR)               result |= PipelineStage::RayTracingShaderKHR;
		if (stage & VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT)         result |= PipelineStage::FragmentDensityProcessEXT;
		if (stage & VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR)	result |= PipelineStage::FragmentShadingRateAttachmentKHR;
		if (stage & VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV)                result |= PipelineStage::CommandPreprocessNV;
		if (stage & VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT)                      result |= PipelineStage::TaskShaderEXT;
		if (stage & VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT)                      result |= PipelineStage::MeshShaderEXT;

        return result;
    }

    VkPipelineStageFlags PipelineStageToVkPipelineStage(PipelineStage stage)
    {
        VkPipelineStageFlags result = 0;

        if (stage & PipelineStage::TopOfPipe)                                   result |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        if (stage & PipelineStage::DrawIndirect)                                result |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
        if (stage & PipelineStage::VertexInput)                                 result |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
        if (stage & PipelineStage::VertexShader)                                result |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        if (stage & PipelineStage::TessellationControlShader)                   result |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
        if (stage & PipelineStage::TessellationEvaluationShader)                result |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
        if (stage & PipelineStage::GeometryShader)                              result |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
        if (stage & PipelineStage::FragmentShader)                              result |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        if (stage & PipelineStage::EarlyFragmentTests)                          result |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        if (stage & PipelineStage::LateFragmentTests)                           result |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        if (stage & PipelineStage::ColourAttachmentOutput)                      result |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        if (stage & PipelineStage::ComputeShader)                               result |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        if (stage & PipelineStage::Transfer)                                    result |= VK_PIPELINE_STAGE_TRANSFER_BIT;
        if (stage & PipelineStage::BottomOfPipe)                                result |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        if (stage & PipelineStage::Host)                                        result |= VK_PIPELINE_STAGE_HOST_BIT;
        if (stage & PipelineStage::AllGraphics)                                 result |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        if (stage & PipelineStage::AllCommands)                                 result |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        if (stage & PipelineStage::TransformFeedbackEXT)                        result |= VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT;
        if (stage & PipelineStage::ConditionalRenderingEXT)                     result |= VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT;
        if (stage & PipelineStage::AccelerationStructureBuildKHR)               result |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
        if (stage & PipelineStage::RayTracingShaderKHR)                         result |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
        if (stage & PipelineStage::FragmentDensityProcessEXT)                   result |= VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT;
        if (stage & PipelineStage::FragmentShadingRateAttachmentKHR)            result |= VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
        if (stage & PipelineStage::CommandPreprocessNV)                         result |= VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV;
        if (stage & PipelineStage::TaskShaderEXT)                               result |= VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT;
        if (stage & PipelineStage::MeshShaderEXT)                               result |= VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT;

        return result;
    }

    CullingMode VkCullModeFlagsToCullingMode(VkCullModeFlags mode)
    {
        switch (mode)
        {
        case VK_CULL_MODE_NONE:                                                 return CullingMode::None;
        case VK_CULL_MODE_FRONT_BIT:                                            return CullingMode::Front;
        case VK_CULL_MODE_BACK_BIT:                                             return CullingMode::Back;
        case VK_CULL_MODE_FRONT_AND_BACK:                                       return CullingMode::FrontAndBack;

        default:
            LU_ASSERT(false, "[VkPipeline] Invalid culling mode passed in.");
            break;
        }

        return CullingMode::None;
    }

    VkCullModeFlags CullingModeToVkCullModeFlags(CullingMode mode)
    {
        switch (mode)
        {
        case CullingMode::None:                                                 return VK_CULL_MODE_NONE;
        case CullingMode::Front:                                                return VK_CULL_MODE_FRONT_BIT;
        case CullingMode::Back:                                                 return VK_CULL_MODE_BACK_BIT;
        case CullingMode::FrontAndBack:                                         return VK_CULL_MODE_FRONT_AND_BACK;

        default:
            LU_ASSERT(false, "[VkPipeline] Invalid culling mode passed in.");
            break;
        }

        return VK_CULL_MODE_NONE;
    }

    PolygonMode VkPolygonModeToPolygonMode(VkPolygonMode mode)
    {
        switch (mode)
        {
        case VK_POLYGON_MODE_FILL:                                              return PolygonMode::Fill;
        case VK_POLYGON_MODE_LINE:                                              return PolygonMode::Line;
        case VK_POLYGON_MODE_POINT:                                             return PolygonMode::Point;
        case VK_POLYGON_MODE_FILL_RECTANGLE_NV:                                 return PolygonMode::FillRectangleNV;

        default:
            LU_ASSERT(false, "[VkPipeline] Invalid polygon mode passed in.");
            break;
        }

        return PolygonMode::None;
    }

    VkPolygonMode PolygonModeToVkPolygonMode(PolygonMode mode)
    {
        switch (mode)
        {
        case PolygonMode::Fill:                                                 return VK_POLYGON_MODE_FILL;
        case PolygonMode::Line:                                                 return VK_POLYGON_MODE_LINE;
        case PolygonMode::Point:                                                return VK_POLYGON_MODE_POINT;
        case PolygonMode::FillRectangleNV:                                      return VK_POLYGON_MODE_FILL_RECTANGLE_NV;

        default:
            LU_ASSERT(false, "[VkPipeline] Invalid polygon mode passed in.");
            break;
        }

        return VK_POLYGON_MODE_MAX_ENUM;
    }

    PipelineBindPoint VkPipelineBindPointToPipelineBindPoint(VkPipelineBindPoint bindPoint)
    {
        switch (bindPoint)
        {
        case VK_PIPELINE_BIND_POINT_GRAPHICS:                                   return PipelineBindPoint::Graphics;
        case VK_PIPELINE_BIND_POINT_COMPUTE:                                    return PipelineBindPoint::Compute;
        case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:                            return PipelineBindPoint::RayTracingKHR;
        case VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI:                     return PipelineBindPoint::SubpassShadingHuawei;

        default:
            LU_ASSERT(false, "[VkPipeline] Invalid pipeline bindpoint passed in.");
            break;
        }

        return PipelineBindPoint::None;
    }

    VkPipelineBindPoint PipelineBindPointToVkPipelineBindPoint(PipelineBindPoint bindPoint)
    {
        switch (bindPoint)
        {
        case PipelineBindPoint::Graphics:                                       return VK_PIPELINE_BIND_POINT_GRAPHICS;
        case PipelineBindPoint::Compute:                                        return VK_PIPELINE_BIND_POINT_COMPUTE;
        case PipelineBindPoint::RayTracingKHR:                                  return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
        case PipelineBindPoint::SubpassShadingHuawei:                           return VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI;

        default:
            LU_ASSERT(false, "[VkPipeline] Invalid pipeline bindpoint passed in.");
            break;
        }

        return VK_PIPELINE_BIND_POINT_MAX_ENUM;
    }

}