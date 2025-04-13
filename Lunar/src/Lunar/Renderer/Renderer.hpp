#pragma once

#include "Lunar/Internal/Renderer/Renderer.hpp"

#include "Lunar/Renderer/RendererSpec.hpp"
#include "Lunar/Renderer/Texture.hpp"
#include "Lunar/Renderer/Batch/BatchRenderer2D.hpp"

namespace Lunar
{

	class Window;

	////////////////////////////////////////////////////////////////////////////////////
	// Renderer
	////////////////////////////////////////////////////////////////////////////////////
	class Renderer
	{
	public:
		// Main methods
		void BeginFrame();
		void EndFrame();

		// 2D methods
		void DrawQuad(const Vec3<float>& position, const Vec2<float>& size, const Vec4<float>& colour);
		void DrawQuad(const Vec3<float>& position, const Vec2<float>& size, Texture& texture, const Vec4<float>& colour = { 1.0f, 1.0f, 1.0f, 1.0f });

		void Set2DCamera(const Mat4& view, const Mat4& projection);

		// Getters
		inline RendererID GetID() const { return static_cast<RendererID>(m_Renderer->GetID()); }

	private:
		// Constructor & Destructor
		Renderer() = default;
		~Renderer() = default;

		// Init & Destroy
		void Init(const Internal::RendererID renderer);
		void Destroy();

		// Private methods
		void Resize(uint32_t width, uint32_t height);

	private:
		Internal::Renderer* m_Renderer = nullptr;

		BatchRenderer2D m_Renderer2D = {};

		friend class Window;
	};

}