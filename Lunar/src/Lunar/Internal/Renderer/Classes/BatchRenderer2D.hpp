#pragma once

#include "Lunar/Internal/Renderer/Image.hpp"
#include "Lunar/Internal/Renderer/Buffers.hpp"
#include "Lunar/Internal/Renderer/Renderer.hpp"
#include "Lunar/Internal/Renderer/Pipeline.hpp"
#include "Lunar/Internal/Renderer/Descriptor.hpp"
#include "Lunar/Internal/Renderer/Renderpass.hpp"
#include "Lunar/Internal/Renderer/CommandBuffer.hpp"

#include "Lunar/Internal/Utils/Settings.hpp"	

#include "Lunar/Maths/Structs.hpp"

#include <cstdint>

namespace Lunar::Internal
{

	class BatchRenderer2D;

	////////////////////////////////////////////////////////////////////////////////////
	// BatchResources2D
	////////////////////////////////////////////////////////////////////////////////////
	class BatchResources2D
	{
	public:

	public:
		struct Vertex
		{
		public:
			Vec3<float> Position = { 0.0f, 0.0f, 0.0f };
			Vec2<float> UV = { 0.0f, 0.0f };
			Vec4<float> Colour = { 1.0f, 1.0f, 1.0f, 1.0f };

			// Note: 0 is the white texure
			uint32_t TextureID = 0; 

		public:
			// Constructors & Destructor
			Vertex() = default;
			Vertex(const Vec3<float>& position, const Vec2<float>& uv, const Vec4<float>& colour, uint32_t textureID)
				: Position(position), UV(uv), Colour(colour), TextureID(textureID) {}
			~Vertex() = default;
		};

	public:
		// Constructor & Destructor
		BatchResources2D() = default;
		~BatchResources2D() = default;

		// Init & Destroy
		void Init(const Internal::RendererID renderer, const std::vector<Image*>& images, LoadOperation loadOperation);
		void Destroy();

		// Methods
		void Resize(uint32_t width, uint32_t height);

	private:
		// Global
		Image m_WhiteTexture;
		UniformBuffer m_CameraBuffer;

		// Renderer
		struct
		{
			std::vector<Image*> Images = { };
			Image DepthImage = {};

			Pipeline Pipeline = {};
			DescriptorSets DescriptorSets = {};

			CommandBuffer CommandBuffer = {};
			Renderpass Renderpass = {};

			VertexBuffer VertexBuffer = {};
			IndexBuffer IndexBuffer = {};
		} Renderer;

		// State
		RendererID m_RendererID = 0;
		std::vector<Vertex> m_CPUBuffer = { };
		
		uint32_t m_CurrentTextureIndex = 0;
		std::unordered_map<Image*, uint32_t> m_TextureIndices = { };

	private:
		// Private methods
		void InitGlobal();
		void InitRenderer(const std::vector<Image*>& images, LoadOperation loadOperation);

		friend class BatchRenderer2D;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// BatchRenderer2D
	////////////////////////////////////////////////////////////////////////////////////
	class BatchRenderer2D
	{
	public:
		constexpr static const uint32_t MaxQuads = 10000u;

		#if !defined(LU_PLATFORM_APPLE)
		constexpr static const uint32_t MaxTextures = 1024u;
		#else // Note: Apple devices have a very small limit of textures, to support most devices we stick with 16. The higher end devices support up to 128 (still low)
		constexpr static const uint32_t MaxTextures = 16u;
		#endif
	public:
		// Constructor & Destructor
		BatchRenderer2D() = default;
		~BatchRenderer2D() = default;

		// Init & Destroy
		void Init(const Internal::RendererID renderer, const std::vector<Image*>& images, LoadOperation loadOperation = LoadOperation::Clear);
		void Destroy();

		// Methods
		void Begin();
		void End();
		void Flush();

		void SetCamera(const Mat4& view, const Mat4& projection);

		// Note: We multiply the Z-axis by -1, so the depth is from 0 to 1
		void AddQuad(const Vec3<float>& position, const Vec2<float>& size, const Vec4<float>& colour);
		void AddQuad(const Vec3<float>& position, const Vec2<float>& size, Image* texture, const Vec4<float>& colour);

		// Internal
		void Resize(uint32_t width, uint32_t height);

	private:
		uint32_t GetTextureID(Image* image);

	private:
		BatchResources2D m_Resources = {};
	};

}