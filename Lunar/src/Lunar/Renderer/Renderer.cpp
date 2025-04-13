#include "lupch.h"
#include "Renderer.hpp"

#include "Lunar/Internal/IO/Print.hpp"

namespace Lunar
{

	////////////////////////////////////////////////////////////////////////////////////
	// Main methods
	////////////////////////////////////////////////////////////////////////////////////
	void Renderer::BeginFrame()
	{
		m_Renderer->BeginFrame();

		// TODO: Renderer3D Begin

		m_Renderer2D.Begin();
	}

	void Renderer::EndFrame()
	{
		// TODO: Renderer3D End/Flush

		m_Renderer2D.End();
		m_Renderer2D.Flush();

		m_Renderer->EndFrame();
		m_Renderer->Present();
	}

	////////////////////////////////////////////////////////////////////////////////////
	// 2D methods
	////////////////////////////////////////////////////////////////////////////////////
	void Renderer::DrawQuad(const Vec3<float>& position, const Vec2<float>& size, const Vec4<float>& colour)
	{
		m_Renderer2D.AddQuad(position, size, colour);
	}

	void Renderer::DrawQuad(const Vec3<float>& position, const Vec2<float>& size, Texture& texture, const Vec4<float>& colour)
	{
		m_Renderer2D.AddQuad(position, size, &texture.m_Image, colour);
	}

	void Renderer::Set2DCamera(const Mat4& view, const Mat4& projection)
	{
		m_Renderer2D.SetCamera(view, projection);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
	void Renderer::Init(const Internal::RendererID renderer)
	{
		m_Renderer = &Internal::Renderer::GetRenderer(renderer);

		auto image = m_Renderer->GetSwapChainImages()[0];
		m_Renderer2D.Init(renderer, image->GetWidth(), image->GetHeight());
	}

	void Renderer::Destroy()
	{
		m_Renderer2D.Destroy();
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Private methods
	////////////////////////////////////////////////////////////////////////////////////
	void Renderer::Resize(uint32_t width, uint32_t height)
	{
		m_Renderer2D.Resize(width, height);
	}
	
}