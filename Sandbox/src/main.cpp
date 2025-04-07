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

#include "Lunar/Internal/Renderer/Image.hpp"
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

layout(location = 0) out vec2 fragUV;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 uViewProjection;
} ubo;

void main() {
    fragUV = inUV;
    gl_Position = ubo.uViewProjection * vec4(inPosition, 0.0, 1.0);
}
)glsl";

const char* g_FragmentShader = R"glsl(
#version 450

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D uTexture;

void main() {
    vec4 texColor = texture(uTexture, fragUV);
    outColor = texColor;
}
)glsl";

struct Vertex 
{
public:
    Lunar::Vec2<float> Position;
    Lunar::Vec2<float> UV;
};

std::vector<Vertex> g_QuadVertices = {
    { {-0.5f, -0.5f}, {0.0f, 0.0f} },         // Bottom-left
    { { 0.5f, -0.5f}, {1.0f, 0.0f} },         // Bottom-right
    { { 0.5f,  0.5f}, {1.0f, 1.0f} },         // Top-right
    { {-0.5f,  0.5f}, {0.0f, 1.0f} },         // Top-left
};

std::vector<uint16_t> g_QuadIndices = {
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
		Lunar::Mat4 ubo = Lunar::Mat4(1.0f);
        Lunar::Internal::UniformBuffer uboBuffer(rendererID, {
            .Usage = Lunar::Internal::BufferMemoryUsage::CPUToGPU,
        }, sizeof(ubo));
		uboBuffer.SetData(rendererID, &ubo, sizeof(ubo));
        Lunar::Internal::Descriptor uboDescriptor = { Lunar::Internal::DescriptorType::UniformBuffer, 0, "ubo", Lunar::Internal::ShaderStage::Vertex };

        // Image
        Lunar::Internal::Image image(rendererID, { 
			.Usage = Lunar::Internal::ImageUsage::Colour | Lunar::Internal::ImageUsage::Sampled,
			.Format = Lunar::Internal::ImageFormat::RGBA,
			.Width = 1,
			.Height = 1,
        }, {});
		size_t colour = 0xFF00FF00;
        image.SetData(rendererID, &colour, sizeof(colour));
		Lunar::Internal::Descriptor imageDescriptor = { Lunar::Internal::DescriptorType::CombinedImageSampler, 1, "uTexture", Lunar::Internal::ShaderStage::Fragment };

        // Descriptors
        Lunar::Internal::DescriptorSets descriptorSets(rendererID, { Lunar::Internal::DescriptorSetRequest({
            .Amount = 1,
            .Layout = Lunar::Internal::DescriptorSetLayout(0, {
                uboDescriptor,
                imageDescriptor
            })
        }) });

        // Shader
        Lunar::Internal::Shader shader(rendererID, {
            .Shaders = {
				{ Lunar::Internal::ShaderStage::Vertex, Lunar::Internal::ShaderCompiler::CompileGLSL(Lunar::Internal::ShaderStage::Vertex, g_VertexShader) },
				{ Lunar::Internal::ShaderStage::Fragment, Lunar::Internal::ShaderCompiler::CompileGLSL(Lunar::Internal::ShaderStage::Fragment, g_FragmentShader) }
            }
        });

        // Renderpass
        Lunar::Internal::CommandBuffer cmdBuf(rendererID);
        Lunar::Internal::Renderpass renderpass(rendererID, Lunar::Internal::RenderpassSpecification({
            .Usage = Lunar::Internal::RenderpassUsage::Graphics,

            .ColourAttachment = renderer.GetSwapChainImages(),
            .ColourClearColour = { 0.1f, 0.1f, 0.1f, 1.0f }
        }), &cmdBuf);

        // Pipeline
		Lunar::Internal::Pipeline pipeline(rendererID, {
            .Bufferlayout = {
                { Lunar::Internal::DataType::Float2, 0, "inPosition"},
                { Lunar::Internal::DataType::Float2, 1, "inUV" },
            }
        }, descriptorSets, shader, renderpass);

        shader.Destroy(rendererID);

		auto set0s = descriptorSets.GetSets(0);
        auto set0 = set0s[0];

        while (window.IsOpen())
        {
            window.PollEvents();
            renderer.BeginFrame();

            set0->Upload(rendererID, { { &uboBuffer, uboDescriptor }, { &image, imageDescriptor } });

            renderer.Begin(renderpass);
            
            pipeline.Use(rendererID, cmdBuf);
            set0->Bind(rendererID, pipeline, cmdBuf);
            vertexBuffer.Bind(rendererID, cmdBuf);
            indexBuffer.Bind(rendererID, cmdBuf);

            renderer.DrawIndexed(cmdBuf, indexBuffer); 

            renderer.End(renderpass);
            renderer.Submit(renderpass, Lunar::Internal::ExecutionPolicy::InOrder);

            renderer.EndFrame();
            renderer.Present();
            window.SwapBuffers();
        }

		vertexBuffer.Destroy(rendererID);
		indexBuffer.Destroy(rendererID);
		uboBuffer.Destroy(rendererID);
        image.Destroy(rendererID);
        descriptorSets.Destroy(rendererID);
		cmdBuf.Destroy(rendererID);
        renderpass.Destroy(rendererID);
        pipeline.Destroy(rendererID);
    }

    return 0;
}