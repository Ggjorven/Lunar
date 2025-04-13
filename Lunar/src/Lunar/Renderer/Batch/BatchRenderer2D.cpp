#include "lupch.h"
#include "BatchRenderer2D.hpp"

#include "Lunar/Internal/IO/Print.hpp"
#include "Lunar/Internal/Utils/Profiler.hpp"

#include "Lunar/Internal/Renderer/Shader.hpp"
#include "Lunar/Internal/Renderer/Renderer.hpp"
#include "Lunar/Internal/Renderer/GraphicsContext.hpp"

#include <array>
#include <string_view>

namespace
{

	////////////////////////////////////////////////////////////////////////////////////
	// Embedded shaders
	////////////////////////////////////////////////////////////////////////////////////
	constexpr const std::string_view s_VertexShader = R"glsl(
		#version 460 core

		layout(location = 0) in vec3 a_Position;
		layout(location = 1) in vec2 a_TexCoord;
		layout(location = 2) in vec4 a_Colour;
		layout(location = 3) in uint a_TextureID;

		layout(location = 0) out vec3 v_Position;
		layout(location = 1) out vec2 v_TexCoord;
		layout(location = 2) out vec4 v_Colour;
		layout(location = 3) flat out uint v_TextureID;

		layout(std140, set = 0, binding = 0) uniform CameraSettings
		{
			mat4 View;
			mat4 Projection;
		} u_Camera;

		void main()
		{
			v_Position = a_Position;
			v_TexCoord = a_TexCoord;
			v_Colour = a_Colour;
			v_TextureID = a_TextureID;

			gl_Position = u_Camera.Projection * u_Camera.View * vec4(a_Position, 1.0);
		}
	)glsl";

	constexpr const std::string_view s_FragmentShader = R"glsl(
		#version 460 core
		#extension GL_KHR_vulkan_glsl : enable
		#extension GL_EXT_nonuniform_qualifier : enable

		layout(location = 0) out vec4 o_Colour;

		layout(location = 0) in vec3 v_Position;
		layout(location = 1) in vec2 v_TexCoord;
		layout(location = 2) in vec4 v_Colour;

		// Note: underlying type is uint16_t, so will never exceed this value.
		layout(location = 3) flat in uint v_TextureID;

		layout (set = 0, binding = 1) uniform sampler2D u_Textures[];

		void main()
		{
			o_Colour = v_Colour * texture(u_Textures[v_TextureID], v_TexCoord);
		}
	)glsl";

	////////////////////////////////////////////////////////////////////////////////////
	// Helper functions
	////////////////////////////////////////////////////////////////////////////////////
	Lunar::Internal::BufferLayout GetVertexBufferLayout()
	{
		return {
			{ Lunar::Internal::DataType::Float3,	0, "Position",	Lunar::Internal::VertexInputRate::Vertex },
			{ Lunar::Internal::DataType::Float2,	1, "UV",		Lunar::Internal::VertexInputRate::Vertex },
			{ Lunar::Internal::DataType::Float4,	2, "Colour",	Lunar::Internal::VertexInputRate::Vertex },
			{ Lunar::Internal::DataType::UInt,		3, "TextureID", Lunar::Internal::VertexInputRate::Vertex },
		};
	}

}

namespace Lunar
{

	////////////////////////////////////////////////////////////////////////////////////
	// Init & Destroy
	////////////////////////////////////////////////////////////////////////////////////
	void BatchResources2D::Init(const Internal::RendererID renderer, uint32_t width, uint32_t height)
	{
		m_RendererID = renderer;

		InitGlobal();
		InitRenderer(width, height);
	}

	void BatchResources2D::Destroy()
	{
		m_WhiteTexture.Destroy(m_RendererID);
		m_CameraBuffer.Destroy(m_RendererID);

		Renderer.DepthImage.Destroy(m_RendererID);

		Renderer.Pipeline.Destroy(m_RendererID);
		Renderer.DescriptorSets.Destroy(m_RendererID);
		
		Renderer.CommandBuffer.Destroy(m_RendererID);
		Renderer.Renderpass.Destroy(m_RendererID);
		
		Renderer.VertexBuffer.Destroy(m_RendererID);
		Renderer.IndexBuffer.Destroy(m_RendererID);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void BatchResources2D::Resize(uint32_t width, uint32_t height)
	{
		Renderer.DepthImage.Resize(m_RendererID, width, height);
		Renderer.Renderpass.Resize(m_RendererID, width, height);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Private methods
	////////////////////////////////////////////////////////////////////////////////////
	void BatchResources2D::InitGlobal()
	{
		m_WhiteTexture.Init(m_RendererID, { 
			.Usage = Internal::ImageUsage::Colour | Internal::ImageUsage::Sampled,
			.Layout = Internal::ImageLayout::ShaderRead,
			.Format = Internal::ImageFormat::RGBA,

			.Width = 1, .Height = 1,

			.MipMaps = false,
		}, {});

		uint32_t white = 0xFFFFFFFF;
		m_WhiteTexture.SetData(m_RendererID, &white, sizeof(uint32_t));

		m_CameraBuffer.Init(m_RendererID, { 
			.Usage = Internal::BufferMemoryUsage::CPUToGPU 
		}, (sizeof(Mat4) * 2)); // For View & Projection

		std::array<Mat4, 2> cameraData = { Mat4(1.0f), Mat4(1.0f) };
		m_CameraBuffer.SetData(m_RendererID, cameraData.data(), sizeof(cameraData));
	}

	void BatchResources2D::InitRenderer(uint32_t width, uint32_t height)
	{
		Internal::Renderer& renderer = Internal::Renderer::GetRenderer(m_RendererID);

		std::vector<uint32_t> indices;
		indices.reserve(static_cast<size_t>(BatchRenderer2D::MaxQuads) * 6);

		for (uint32_t i = 0, offset = 0; i < BatchRenderer2D::MaxQuads * 6; i += 6, offset += 4)
		{
			indices.push_back(offset + 0);
			indices.push_back(offset + 1);
			indices.push_back(offset + 2);

			indices.push_back(offset + 2);
			indices.push_back(offset + 3);
			indices.push_back(offset + 0);
		}

		// Depth image
		Renderer.DepthImage.Init(m_RendererID, {
			.Usage = Internal::ImageUsage::DepthStencil | Internal::ImageUsage::Sampled,
			.Layout = Internal::ImageLayout::DepthStencil,
			.Format = renderer.GetDepthFormat(),
			
			.Width = width, .Height = height,

			.MipMaps = false,
		}, {});

		// Renderpass
		Renderer.CommandBuffer.Init(m_RendererID);
		Renderer.Renderpass.Init(m_RendererID, {
			.Usage = Internal::RenderpassUsage::Graphics,

			.ColourAttachment = renderer.GetSwapChainImages(),
			.ColourLoadOp = Internal::LoadOperation::Clear,		// TODO: Set this to LoadOp::Load when also using 3D rendering and set that renderpass to Clear and run that first and then run this renderpass
			.ColourStoreOp = Internal::StoreOperation::Store,
			.ColourClearColour { 0.0f, 0.0f, 0.0f, 1.0f },
			.PreviousColourImageLayout = Internal::ImageLayout::Undefined,
			.FinalColourImageLayout = Internal::ImageLayout::PresentSrcKHR,

			.DepthAttachment = &Renderer.DepthImage,
			.DepthLoadOp = Internal::LoadOperation::Clear,
			.DepthStoreOp = Internal::StoreOperation::Store,
			.PreviousDepthImageLayout = Internal::ImageLayout::Undefined,
			.FinalDepthImageLayout = Internal::ImageLayout::DepthStencil,
		}, &Renderer.CommandBuffer);

		// Shader
		Internal::Shader shader(m_RendererID, {
			.Shaders = {
				{ Internal::ShaderStage::Vertex, Internal::ShaderCompiler::CompileGLSL(Internal::ShaderStage::Vertex, s_VertexShader.data()) },
				{ Internal::ShaderStage::Fragment, Internal::ShaderCompiler::CompileGLSL(Internal::ShaderStage::Fragment, s_FragmentShader.data()) }
			}
		});

		// Descriptorsets
		Renderer.DescriptorSets.Init(m_RendererID, {
			{ 1, { 0, {
				{ Internal::DescriptorType::UniformBuffer, 0, "u_Camera", Internal::ShaderStage::Vertex },
				{ Internal::DescriptorType::CombinedImageSampler, 1, "u_Textures", Internal::ShaderStage::Fragment, BatchRenderer2D::MaxTextures, Internal::DescriptorBindingFlags::Default },
			}}}
		});

		// Pipeline
		Renderer.Pipeline.Init(m_RendererID, {
			.Usage = Internal::PipelineUsage::Graphics,
			.Bufferlayout = GetVertexBufferLayout(),
			.Polygonmode = Internal::PolygonMode::Fill,
			.Cullingmode = Internal::CullingMode::None,
			.Blending = true
		}, Renderer.DescriptorSets, shader, Renderer.Renderpass);
		shader.Destroy(m_RendererID);

		// Buffers
		Renderer.VertexBuffer.Init(m_RendererID, { 
			.Usage = Internal::BufferMemoryUsage::CPUToGPU 
		}, nullptr, sizeof(BatchResources2D::Vertex) * BatchRenderer2D::MaxQuads * 4);
		Renderer.IndexBuffer.Init(m_RendererID, {}, indices.data(), static_cast<uint32_t>(indices.size()));

		// Reserve enough space for cpu buffer
		m_CPUBuffer.reserve(static_cast<size_t>(BatchRenderer2D::MaxQuads) * 4);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Init & Destroy
	////////////////////////////////////////////////////////////////////////////////////
	void BatchRenderer2D::Init(const Internal::RendererID renderer, uint32_t width, uint32_t height)
	{
		#if defined(LU_PLATFORM_APPLE)
		LU_LOG_WARN("[BatchRenderer2D] BatchRenderer2D only supports {0} simultaneous textures on apple devices. Should be used with care.", MaxTextures);
		#endif
		m_Resources.Init(renderer, width, height);
	}

	void BatchRenderer2D::Destroy()
	{
		m_Resources.Destroy();
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void BatchRenderer2D::Begin()
	{
		LU_PROFILE("BatchRenderer2D::Begin()");
		m_Resources.m_CPUBuffer.clear();

		m_Resources.m_CurrentTextureIndex = 0;
		m_Resources.m_TextureIndices.clear(); // TODO: Maybe optimize

		// Set the white texture to index 0
		m_Resources.m_TextureIndices[&m_Resources.m_WhiteTexture] = m_Resources.m_CurrentTextureIndex++;
	}

	void BatchRenderer2D::End()
	{
		LU_PROFILE("BatchRenderer2D::End()");
		std::vector<Internal::Uploadable> uploadQueue;
		uploadQueue.reserve(m_Resources.m_TextureIndices.size() + 1); // + 1 for the Camera Buffer.
		{
			LU_PROFILE("BatchRenderer2D::End::FormUploadQueue");
			uploadQueue.push_back({ &m_Resources.m_CameraBuffer, m_Resources.Renderer.DescriptorSets.GetLayout(0).GetDescriptorByName("u_Camera") });

			// Upload all images
			const auto& descriptor = m_Resources.Renderer.DescriptorSets.GetLayout(0).GetDescriptorByName("u_Textures");
			for (const auto& [image, textureID] : m_Resources.m_TextureIndices)
				uploadQueue.push_back({ image, descriptor, textureID });
		}
		// Actual upload command
		{
			LU_PROFILE("BatchRenderer2D::End::ExecuteUploadQueue");
			m_Resources.Renderer.DescriptorSets.GetSets(0)[0]->Upload(m_Resources.m_RendererID, uploadQueue);
		}

		// Only draw if there's something TO draw
		if (m_Resources.m_CPUBuffer.size() > 0)
		{
			LU_PROFILE("BatchRenderer2D::End::UploadVertexData");
			m_Resources.Renderer.VertexBuffer.SetData(m_Resources.m_RendererID, (void*)m_Resources.m_CPUBuffer.data(), (m_Resources.m_CPUBuffer.size() * sizeof(BatchResources2D::Vertex)), 0);
		}
	}

	void BatchRenderer2D::Flush()
	{
		LU_PROFILE("BatchRenderer2D::Flush()");

		Internal::Renderer& renderer = Internal::Renderer::GetRenderer(m_Resources.m_RendererID);

		// Start rendering
		renderer.Begin(m_Resources.Renderer.Renderpass);

		Internal::CommandBuffer& cmdBuf = m_Resources.Renderer.Renderpass.GetCommandBuffer();
		m_Resources.Renderer.Pipeline.Use(m_Resources.m_RendererID, cmdBuf, Internal::PipelineBindPoint::Graphics);

		m_Resources.Renderer.DescriptorSets.GetSets(0)[0]->Bind(m_Resources.m_RendererID, m_Resources.Renderer.Pipeline, cmdBuf);

		m_Resources.Renderer.IndexBuffer.Bind(m_Resources.m_RendererID, cmdBuf);
		m_Resources.Renderer.VertexBuffer.Bind(m_Resources.m_RendererID, cmdBuf);

		// Draw all at once
		renderer.DrawIndexed(cmdBuf, static_cast<uint32_t>(((m_Resources.m_CPUBuffer.size() / 4ull) * 6ull)), 1);

		// End rendering
		renderer.End(m_Resources.Renderer.Renderpass);
		renderer.Submit(m_Resources.Renderer.Renderpass, Internal::ExecutionPolicy::InOrder);
	}

	void BatchRenderer2D::SetCamera(const Mat4& view, const Mat4& projection)
	{
		std::array<Mat4, 2> cameraData = { view, projection };
		m_Resources.m_CameraBuffer.SetData(m_Resources.m_RendererID, cameraData.data(), sizeof(cameraData));
	}

	void BatchRenderer2D::AddQuad(const Vec3<float>& position, const Vec2<float>& size, const Vec4<float>& colour)
	{
		AddQuad(position, size, nullptr, colour);
	}

	void BatchRenderer2D::AddQuad(const Vec3<float>& position, const Vec2<float>& size, Internal::Image* texture, const Vec4<float>& colour)
	{
		LU_PROFILE("BatchRenderer2D::AddQuad()");
		constexpr const Vec2<float> uv0(1.0f, 0.0f);
		constexpr const Vec2<float> uv1(0.0f, 0.0f);
		constexpr const Vec2<float> uv2(0.0f, 1.0f);
		constexpr const Vec2<float> uv3(1.0f, 1.0f);

		#if !defined(LU_CONFIG_DIST)
		if ((m_Resources.m_CPUBuffer.size() / 4u) >= BatchRenderer2D::MaxQuads) [[unlikely]]
		{
			LU_LOG_WARN("[BatchRenderer2D] Reached max amount of quads ({0}), to support more either manually change BatchRenderer2D::MaxQuads or contact the developer.", BatchRenderer2D::MaxQuads);
			return;
		}
		#endif

		const uint32_t textureID = GetTextureID(texture);
		const float zAxis = position.z * -1.0f;

		m_Resources.m_CPUBuffer.emplace_back(Vec3<float>(position.x, position.y, zAxis), uv0, colour, textureID);
		m_Resources.m_CPUBuffer.emplace_back(Vec3<float>(position.x + size.x, position.y, zAxis), uv1, colour, textureID);
		m_Resources.m_CPUBuffer.emplace_back(Vec3<float>(position.x + size.x, position.y + size.y, zAxis), uv2, colour, textureID);
		m_Resources.m_CPUBuffer.emplace_back(Vec3<float>(position.x, position.y + size.y, zAxis), uv3, colour, textureID);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Internal
	////////////////////////////////////////////////////////////////////////////////////
	void BatchRenderer2D::Resize(uint32_t width, uint32_t height)
	{
		m_Resources.Resize(width, height);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Private methods
	////////////////////////////////////////////////////////////////////////////////////
	uint32_t BatchRenderer2D::GetTextureID(Internal::Image* image)
	{
		// If nullptr return white texture
		if (image == nullptr)
			return m_Resources.m_TextureIndices[&m_Resources.m_WhiteTexture];

		// Check if texture is has not been cached
		if (m_Resources.m_TextureIndices.find(image) == m_Resources.m_TextureIndices.end())
		{
			#if !defined(LU_CONFIG_DIST)
			if (m_Resources.m_CurrentTextureIndex >= (BatchRenderer2D::MaxTextures - 1)) [[unlikely]]
			{
				LU_LOG_WARN("[BatchRenderer2D] Reached max amount of textures ({0}), to support more either manually change BatchRenderer2D::MaxTextures or contact the developer. Be aware that apple devices have a very low hardware-set limit.", BatchRenderer2D::MaxTextures);
				return 0;
			}
			#endif

			m_Resources.m_TextureIndices[image] = m_Resources.m_CurrentTextureIndex++;
		}

		return m_Resources.m_TextureIndices[image];
	}

}