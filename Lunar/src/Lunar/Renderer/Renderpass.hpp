#pragma once

#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/Renderer/Classes/BatchRenderer2D.hpp"

#include "Lunar/Renderer/RendererSpec.hpp"
#include "Lunar/Renderer/Texture.hpp"

#include <cstdint>
#include <filesystem>

namespace Lunar
{

	////////////////////////////////////////////////////////////////////////////////////
	// Selection
	////////////////////////////////////////////////////////////////////////////////////
	template<Internal::Info::Platform API>
	struct Renderpass2DRendererSelect;

	template<> struct Renderpass2DRendererSelect<Internal::Info::Platform::Windows> { using Type = Internal::BatchRenderer2D; };
	template<> struct Renderpass2DRendererSelect<Internal::Info::Platform::Linux> { using Type = Internal::BatchRenderer2D; };
	template<> struct Renderpass2DRendererSelect<Internal::Info::Platform::MacOS> { using Type = Internal::BatchRenderer2D; }; // TODO: Create a different renderer for MacOS

	using Renderpass2DRendererType = typename Renderpass2DRendererSelect<Internal::Info::g_Platform>::Type;

	enum class LoadOperation : uint8_t
	{
		Clear = 0,
		Load,
	};

	////////////////////////////////////////////////////////////////////////////////////
	// Renderpass2D
	////////////////////////////////////////////////////////////////////////////////////
	class Renderpass2D
	{
	public:
		// Constructor & Destructor
		Renderpass2D() = default;
		Renderpass2D(const RendererID renderer, LoadOperation loadOperation = LoadOperation::Clear) { Init(renderer, loadOperation); }
		Renderpass2D(const RendererID renderer, Texture& texture, LoadOperation loadOperation = LoadOperation::Clear) { Init(renderer, texture, loadOperation); }
		~Renderpass2D();

		// Init & Destroy
		void Init(const RendererID renderer, LoadOperation loadOperation = LoadOperation::Clear);
		void Init(const RendererID renderer, Texture& texture, LoadOperation loadOperation = LoadOperation::Clear);

		// Methods
		void Begin();
		void End();

		void DrawQuad(const Vec3<float>& position, const Vec2<float>& size, const Vec4<float>& colour);
		void DrawQuad(const Vec3<float>& position, const Vec2<float>& size, Texture& texture, const Vec4<float>& colour = { 1.0f, 1.0f, 1.0f, 1.0f });

		void Set2DCamera(const Mat4& view, const Mat4& projection);

		// Other methods
		void Resize(uint32_t width, uint32_t height);

	private:
		RendererID m_RendererID = 0;
		Renderpass2DRendererType m_Renderer2D = {};

		friend class Renderer;
	};

}