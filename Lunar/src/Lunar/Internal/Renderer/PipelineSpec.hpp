#pragma once

#include "Lunar/Internal/Enum/Bitwise.hpp"

#include "Lunar/Internal/Renderer/ImageSpec.hpp"

#include <unordered_map>

namespace Lunar::Internal
{

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Specification
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Note: Used in Renderer::Submit as a waitStage
	enum class PipelineStage : uint32_t
    {
        None = 0,
        TopOfPipe = 1 << 0,
        DrawIndirect = 1 << 1,
        VertexInput = 1 << 2,
        VertexShader = 1 << 3,
        TessellationControlShader = 1 << 4,
        TessellationEvaluationShader = 1 << 5,
        GeometryShader = 1 << 6,
        FragmentShader = 1 << 7,
        EarlyFragmentTests = 1 << 8,
        LateFragmentTests = 1 << 9,
        ColourAttachmentOutput = 1 << 10,
        ComputeShader = 1 << 11,
        Transfer = 1 << 12,
        BottomOfPipe = 1 << 12,
        Host = 1 << 13,
        AllGraphics = 1 << 14,
        AllCommands = 1 << 15,
        TransformFeedbackEXT = 1 << 16,
        ConditionalRenderingEXT = 1 << 17,
        AccelerationStructureBuildKHR = 1 << 18,
        RayTracingShaderKHR = 1 << 19,
        FragmentDensityProcessEXT = 1 << 20,
        FragmentShadingRateAttachmentKHR = 1 << 21,
        CommandPreprocessNV = 1 << 22,
        TaskShaderEXT = 1 << 23,
        MeshShaderEXT = 1 << 24,

        ShadingRateImageNV = FragmentShadingRateAttachmentKHR,
        RayTracingShaderNV = RayTracingShaderKHR,
        AccelerationStructureBuildNV = AccelerationStructureBuildKHR,
        TaskShaderNV = TaskShaderEXT,
        MeshShaderNV = MeshShaderEXT,
        NoneKHR = None,
    };
    LU_ENABLE_BITWISE(PipelineStage);

    enum class CullingMode : uint8_t
    {
        None = 0,
        Front,
        Back,
        FrontAndBack
    };

    enum class PolygonMode : uint8_t
    {
        None = 0,
        Fill,
        Line,
        Point,
        FillRectangleNV,
    };

    enum class PipelineType : uint8_t
    {
        Graphics = 0,
        Compute,
        RayTracingKHR,
        RayTracingNV
    };

    /*
    // Global specification for push constants
    struct PushConstantsSpecification
    {
    public:
        size_t Offset = 0;
        size_t Size = 0;
    };

    struct PipelineSpecification
    {
    public:
        PipelineType Type = PipelineType::Graphics;

        // Note: Keep in mind that pushConstants most of the time only have a size of 128 bytes (two mat4's).
        // Note 2: Here's an example of how to use pushConstants across multiple shader stages:
        // pipelineSpecification.PushConstants[ShaderStage::Vertex | ShaderStage::Fragment] = {};
        std::unordered_map<ShaderStage, PushConstantsSpecification> PushConstants = { };

        // Graphics
        BufferLayout Bufferlayout = {};

        PolygonMode Polygonmode = PolygonMode::Fill;
        CullingMode Cullingmode = CullingMode::Front; // Note: Before blaming the culling mode, make sure you have depth in your renderpass ;)

        float LineWidth = 1.0f; // Note: Don't use, since for compatibility reasons we have disabled PhysicalDeviceFeature: WideLines.
        bool Blending = false;

        // Dynamic rendering
        ImageFormat DynamicColourFormat = ImageFormat::BGRA;
        ImageFormat DynamicDepthFormat = ImageFormat::Depth32SFloat;
        ImageFormat DynamicStencilFormat = ImageFormat::Undefined;

        // Raytracing KHR
        uint32_t MaxRayRecursion = 1;
    };
	*/

}