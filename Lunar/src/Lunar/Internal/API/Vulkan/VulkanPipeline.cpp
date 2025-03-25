#include "lupch.h"
#include "VulkanPipeline.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanDevice.hpp"

namespace Lunar::Internal
{

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

}