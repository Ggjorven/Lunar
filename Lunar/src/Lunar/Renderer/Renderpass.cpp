#include "lupch.h"
#include "Renderpass.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include "Lunar/Internal/Renderer/Renderer.hpp"

#include "Lunar/Renderer/Renderer.hpp"

namespace Lunar
{

	namespace
	{

		Internal::LoadOperation LoadOperationToInternalLoadOperation(LoadOperation loadOperation)
		{
			switch (loadOperation)
			{
			case LoadOperation::Clear:			return Internal::LoadOperation::Clear;
			case LoadOperation::Load:			return Internal::LoadOperation::Load;
			}

			LU_ASSERT(false, "[Renderpass] Invalid LoadOperation passed in.");
			return Internal::LoadOperation::None;
		}

	}

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
	Renderpass2D::~Renderpass2D()
	{
		m_Renderer2D.Destroy();
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Init & Destroy
	////////////////////////////////////////////////////////////////////////////////////
	void Renderpass2D::Init(const RendererID renderer, LoadOperation loadOperation)
	{
		m_RendererID = renderer;

		Renderer::GetRenderer(renderer).AddPass(this);

		Internal::Renderer& rendererObj = Internal::Renderer::GetRenderer(static_cast<Internal::RendererID>(m_RendererID));
		m_Renderer2D.Init(renderer, rendererObj.GetSwapChainImages(), LoadOperationToInternalLoadOperation(loadOperation));
	}

	void Renderpass2D::Init(const RendererID renderer, Texture& texture, LoadOperation loadOperation)
	{
		LU_ASSERT(false, "[Renderpass2D] Renderpass2D::Init() with texture is not supported yet."); // TODO: Implement proper texture rendering with proper image layouts as previous and final
		m_RendererID = renderer;
		m_Renderer2D.Init(renderer, { &texture.m_Image }, LoadOperationToInternalLoadOperation(loadOperation));
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void Renderpass2D::Begin()
	{
		m_Renderer2D.Begin();
	}

	void Renderpass2D::End()
	{
		m_Renderer2D.End();
		m_Renderer2D.Flush();
	}

	void Renderpass2D::DrawQuad(const Vec3<float>& position, const Vec2<float>& size, const Vec4<float>& colour)
	{
		m_Renderer2D.AddQuad(position, size, colour);
	}

	void Renderpass2D::DrawQuad(const Vec3<float>& position, const Vec2<float>& size, Texture& texture, const Vec4<float>& colour)
	{
		m_Renderer2D.AddQuad(position, size, &texture.m_Image, colour);
	}

	void Renderpass2D::Set2DCamera(const Mat4& view, const Mat4& projection)
	{
		m_Renderer2D.SetCamera(view, projection);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void Renderpass2D::Resize(uint32_t width, uint32_t height)
	{
		m_Renderer2D.Resize(width, height);
	}

}