///////////////////////////////////////////////////////////
// Build test file
///////////////////////////////////////////////////////////
#include "Lunar/Internal/Core/Events.hpp"
#include "Lunar/Internal/Core/Window.hpp"

#include "Lunar/Internal/Enum/Bitwise.hpp"
#include "Lunar/Internal/Enum/Fuse.hpp"
#include "Lunar/Internal/Enum/Name.hpp"
#include "Lunar/Internal/Enum/Utilities.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include "Lunar/Internal/Maths/Logarithm.hpp"
#include "Lunar/Internal/Maths/Structs.hpp"

#include "Lunar/Internal/Memory/Arc.hpp"
#include "Lunar/Internal/Memory/AutoRelease.hpp"
#include "Lunar/Internal/Memory/Box.hpp"
#include "Lunar/Internal/Memory/Rc.hpp"

#include "Lunar/Internal/Renderer/Shader.hpp"
#include "Lunar/Internal/Renderer/Renderer.hpp"
#include "Lunar/Internal/Renderer/Descriptor.hpp"

#include "Lunar/Internal/Utils/Hash.hpp"
#include "Lunar/Internal/Utils/Preprocessor.hpp"
#include "Lunar/Internal/Utils/Profiler.hpp"
#include "Lunar/Internal/Utils/Settings.hpp"
#include "Lunar/Internal/Utils/Types.hpp"

#include "Lunar/Enum/Name.hpp"
#include "Lunar/Enum/Fuse.hpp"

#include "Lunar/Maths/Structs.hpp"

///////////////////////////////////////////////////////////
// Test Enum
///////////////////////////////////////////////////////////
enum class Test : uint8_t
{
    Hi = 0,
    Hello = 27,
    Hey,
};

///////////////////////////////////////////////////////////
// Renderer Test
///////////////////////////////////////////////////////////
const char* g_VertexShader = R"glsl(
#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec4 fragColor;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 uViewProjection;
} ubo;

void main() {
    fragUV = inUV;
    fragColor = inColor;
    gl_Position = ubo.uViewProjection * vec4(inPosition, 0.0, 1.0);
}
)glsl";

const char* g_FragmentShader = R"glsl(
#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = fragColor;
}
)glsl";

struct Vertex 
{
public:
    glm::vec2 Position;
    glm::vec2 UV;
    glm::vec4 Colour;
};

std::vector<Vertex> g_QuadVertices = {
    {{-0.5f, -0.5f}, {0.0f, 0.0f}, {1, 0, 0, 1}}, // Bottom-left
    {{ 0.5f, -0.5f}, {1.0f, 0.0f}, {0, 1, 0, 1}}, // Bottom-right
    {{ 0.5f,  0.5f}, {1.0f, 1.0f}, {0, 0, 1, 1}}, // Top-right
    {{-0.5f,  0.5f}, {0.0f, 1.0f}, {1, 1, 0, 1}}, // Top-left
};

std::vector<uint32_t> g_QuadIndices = {
    0, 1, 2, 2, 3, 0
};

///////////////////////////////////////////////////////////
// Test main
///////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // Enum Test
    constexpr const Test compValue = Test::Hello;
    volatile const Test runtimeValue = Test::Hey;

    constexpr std::string_view compValueStr = Lunar::Enum::Name(compValue);
    LU_LOG_TRACE("Compile time: {0}", compValueStr);
    LU_LOG_TRACE("Runtime: {0}", Lunar::Enum::Name(runtimeValue));

    // Types Test
	constexpr std::string_view typeName = Lunar::Internal::Types::ConstexprName<Test>::TypeName;
	LU_LOG_TRACE("Type name: {0}", typeName);

    // Window Test
    Lunar::Internal::Window window({
        .Title = "Window",

        .Width = 1280,
        .Height = 720,

        .EventCallback = [](Lunar::Internal::Event) {}
    });

    window.GetSpecification().EventCallback = [&](Lunar::Internal::Event e)
    {
        Lunar::Internal::EventHandler handler(e);
        handler.Handle<Lunar::Internal::WindowResizeEvent>([&](Lunar::Internal::WindowResizeEvent& wre) { window.Resize(wre.GetWidth(), wre.GetHeight()); });
        handler.Handle<Lunar::Internal::WindowCloseEvent>([&](Lunar::Internal::WindowCloseEvent&) { window.Close(); });
    };

	Lunar::Internal::Renderer& renderer = window.GetRenderer();
    Lunar::Internal::RendererID rendererID = renderer.GetID();

	// Renderer Test
    {
        // Buffers
		Lunar::Internal::VertexBuffer vertexBuffer(rendererID, {
			.Usage = Lunar::Internal::BufferMemoryUsage::GPU,
        }, g_QuadVertices.data(), sizeof(Vertex)* g_QuadVertices.size());
		Lunar::Internal::IndexBuffer indexBuffer(rendererID, {
			.Usage = Lunar::Internal::BufferMemoryUsage::GPU,
		}, g_QuadIndices.data(), static_cast<uint32_t>(g_QuadIndices.size()));

        // Uniform Buffer
		glm::mat4 ubo = glm::mat4(1.0f);
        Lunar::Internal::UniformBuffer uboBuffer(rendererID, {
            .Usage = Lunar::Internal::BufferMemoryUsage::CPUToGPU,
        }, sizeof(ubo));
		uboBuffer.SetData(&ubo, sizeof(ubo));

        // Descriptors
        Lunar::Internal::DescriptorSets descriptorSets(rendererID, { Lunar::Internal::DescriptorSetRequest({
            .Amount = 1,
            .Layout = Lunar::Internal::DescriptorSetLayout(0, {
                { Lunar::Internal::DescriptorType::UniformBuffer, 0, "ubo", Lunar::Internal::ShaderStage::Vertex },
            })
        }) });

        // Shader
        Lunar::Internal::ShaderSpecification shaderSpecs = {};
        shaderSpecs.Shaders[Lunar::Internal::ShaderStage::Vertex] = Lunar::Internal::ShaderCompiler::CompileGLSL(Lunar::Internal::ShaderStage::Vertex, g_VertexShader);
        shaderSpecs.Shaders[Lunar::Internal::ShaderStage::Fragment] = Lunar::Internal::ShaderCompiler::CompileGLSL(Lunar::Internal::ShaderStage::Fragment, g_FragmentShader);
		Lunar::Internal::Shader shader(rendererID, shaderSpecs);

        // Renderpass
        Lunar::Internal::CommandBuffer cmdBuf(rendererID);
        Lunar::Internal::Renderpass renderpass(rendererID, Lunar::Internal::RenderpassSpecification({
            .Usage = Lunar::Internal::RenderpassUsage::Forward,

            .ColourAttachment = renderer.GetSwapChainImages(),
            .ColourClearColour = { 0.1f, 0.1f, 0.1f, 1.0f }
        }), &cmdBuf);

        // Pipeline
        Lunar::Internal::PipelineSpecification pipelineSpecs = {
            .Bufferlayout = { 
                { Lunar::Internal::DataType::Float2, 0, "inPosition"},
				{ Lunar::Internal::DataType::Float2, 1, "inUV" },
				{ Lunar::Internal::DataType::Float4, 2, "inColor" }
            }
        };
		Lunar::Internal::Pipeline pipeline(rendererID, pipelineSpecs, descriptorSets, shader, renderpass);

        while (window.IsOpen())
        {
            window.PollEvents();
            renderer.BeginFrame();

            descriptorSets.GetSets(0)[0]->Upload({ { &uboBuffer, descriptorSets.GetLayout(0).GetDescriptorByName("ubo")}});

            renderer.Begin(renderpass);
            
            pipeline.Use(cmdBuf);
			descriptorSets.GetSets(0)[0]->Bind(pipeline, cmdBuf);
            vertexBuffer.Bind(cmdBuf);
            indexBuffer.Bind(cmdBuf);

            renderer.DrawIndexed(cmdBuf, indexBuffer); 

            renderer.End(renderpass);
            renderer.Submit(renderpass, Lunar::Internal::ExecutionPolicy::InOrder);

            renderer.EndFrame();
            renderer.Present();
            window.SwapBuffers();
        }
    }

    return 0;
}