#pragma once

#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/Renderer/RendererSpec.hpp"
#include "Lunar/Internal/Renderer/PipelineSpec.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanPipeline.hpp"

namespace Lunar::Internal
{

    class Shader;
    class Renderpass;
    class DescriptorSets;

    ////////////////////////////////////////////////////////////////////////////////////
    // Selection
    ////////////////////////////////////////////////////////////////////////////////////
    template<Info::RenderingAPI API>
    struct PipelineSelect;

    template<> struct PipelineSelect<Info::RenderingAPI::Vulkan> { using Type = VulkanPipeline; };

    using PipelineType = typename PipelineSelect<Info::g_RenderingAPI>::Type;

    ////////////////////////////////////////////////////////////////////////////////////
    // Pipeline
    ////////////////////////////////////////////////////////////////////////////////////
    class Pipeline // Note: Pipeline does not keep the shader/descriptorsets/renderpass alive/in scope
    {
    public:
        // Constructor & Destructor
        inline Pipeline(const RendererID renderer, const PipelineSpecification& specs, DescriptorSets& sets, Shader& shader) { m_Pipeline.Init(renderer, specs, sets, shader); }
        inline Pipeline(const RendererID renderer, const PipelineSpecification& specs, DescriptorSets& sets, Shader& shader, Renderpass& renderpass) { m_Pipeline.Init(renderer, specs, sets, shader, renderpass); }
        inline ~Pipeline() { m_Pipeline.Destroy(); }

        // Methods
        inline void Use(CommandBuffer& cmdBuf, PipelineBindPoint bindPoint = PipelineBindPoint::Graphics) { m_Pipeline.Use(cmdBuf, bindPoint); }

        // Push entire constant of stage specified in PipelineSpecification.PushConstants
        inline void PushConstant(CommandBuffer& cmdBuf, ShaderStage stage, void* data) { m_Pipeline.PushConstant(cmdBuf, stage, data); }
        // Push part of a constant manually
        inline void PushConstant(CommandBuffer& cmdBuf, ShaderStage stage, void* data, size_t offset, size_t size) { m_Pipeline.PushConstant(cmdBuf, stage, data, offset, size); }

        // Make sure a compute shader is present in the current pipeline and that the pipeline is bound.
        inline void DispatchCompute(CommandBuffer& cmdBuf, uint32_t width, uint32_t height, uint32_t depth) { m_Pipeline.DispatchCompute(cmdBuf, width, height, depth); }

        // Getters
        inline const PipelineSpecification& GetSpecification() const { return m_Pipeline.GetSpecification(); }

    private:
        PipelineType m_Pipeline = {};
    };

}