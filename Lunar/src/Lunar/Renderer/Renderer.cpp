#include "lupch.h"
#include "Renderer.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include "Lunar/Renderer/Renderpass.hpp"

#include <unordered_map>

namespace Lunar
{

	namespace
	{
		static std::array<Renderer*, std::numeric_limits<RendererID>::max()> s_Renderers = { };
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Main methods
	////////////////////////////////////////////////////////////////////////////////////
	void Renderer::BeginFrame()
	{
		m_Renderer->BeginFrame();
	}

	void Renderer::EndFrame()
	{
		m_Renderer->EndFrame();
		m_Renderer->Present();
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
	void Renderer::Init(const Internal::RendererID renderer)
	{
		s_Renderers[renderer] = this;
		m_Renderer = &Internal::Renderer::GetRenderer(renderer);
	}

	void Renderer::Destroy()
	{
		s_Renderers[m_Renderer->GetID()] = nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Private methods
	////////////////////////////////////////////////////////////////////////////////////
	void Renderer::Resize(uint32_t width, uint32_t height)
	{
		for (auto& pass2D : m_2DPasses)
			pass2D->Resize(width, height);
	}

	void Renderer::AddPass(Renderpass2D* pass)
	{
		m_2DPasses.push_back(pass);
	}

	void Renderer::RemovePass(Renderpass2D* pass) // TODO: Check functionality
	{
		auto it = std::remove(m_2DPasses.begin(), m_2DPasses.end(), pass);
		if (it != m_2DPasses.end())
			m_2DPasses.erase(it);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Static methods
	////////////////////////////////////////////////////////////////////////////////////
	Renderer& Renderer::GetRenderer(const RendererID renderer)
	{
		return *s_Renderers[renderer];
	}
	
}