#include "lupch.h"
#include "Renderer.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include <array>
#include <limits>

namespace Lunar::Internal
{

    namespace
    {
        static RendererID s_CurrentID = 0;
        static std::array<Renderer*, std::numeric_limits<RendererID>::max()> s_Renderers = { };
    }

    ////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
	Renderer::Renderer()
		: m_ID(s_CurrentID++)
    {
		s_Renderers[m_ID] = this;
    }

    Renderer::~Renderer()
    {
        s_Renderers[m_ID] = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Static methods
    ////////////////////////////////////////////////////////////////////////////////////
    Renderer& Renderer::GetRenderer(RendererID id)
    {
        return *s_Renderers[id];
    }

}