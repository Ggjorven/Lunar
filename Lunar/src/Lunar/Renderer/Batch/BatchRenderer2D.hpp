#pragma once

#include "Lunar/Internal/Renderer/Image.hpp"
#include "Lunar/Internal/Renderer/Buffers.hpp"
#include "Lunar/Internal/Renderer/Renderer.hpp"
#include "Lunar/Internal/Renderer/Pipeline.hpp"
#include "Lunar/Internal/Renderer/Renderpass.hpp"
#include "Lunar/Internal/Renderer/CommandBuffer.hpp"

#include "Lunar/Maths/Structs.hpp"

#include <cstdint>

namespace Lunar
{

	class BatchRenderer2D;

	////////////////////////////////////////////////////////////////////////////////////
	// BatchResources2D
	////////////////////////////////////////////////////////////////////////////////////
	class BatchResources2D
	{
	public:
		struct Vertex
		{
		public:
			Vec3<float> Position = { 0.0f, 0.0f, 0.0f };
			Vec2<float> UV = { 0.0f, 0.0f };
			Vec4<float> Colour = { 1.0f, 1.0f, 1.0f, 1.0f };

			// Note: 0 is the white texure
			uint32_t TextureID = 0; 
		};

	public:
		// Constructor & Destructor
		//BatchResources2D();
		//~BatchResources2D();

	private:
		struct
		{
			Lunar::Internal::Image WhiteTexture;
			Lunar::Internal::UniformBuffer CameraBuffer;
		} Global;

		struct
		{
			uint32_t a;
		} Renderer;

		struct
		{
			uint32_t b;
		} State;

	private:
		// Private methods
		void InitGlobal();
		void InitRenderer();

		friend class BatchRenderer2D;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// BatchRenderer2D
	////////////////////////////////////////////////////////////////////////////////////
	class BatchRenderer2D
	{
	public:
		//BatchRenderer2D();
		//~BatchRenderer2D();

	private:
		BatchResources2D m_Resources;
	};

}