#include "lupch.h"
#include "GraphicsContext.hpp"

#include "Lunar/Internal/IO/Print.hpp"

namespace Lunar::Internal
{

    namespace
    {
        static bool s_Initialized = false;
        static ContextSelect<Info::g_RenderingAPI>::Type s_GraphicsContext = {};

        static void* s_ActiveWindow = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Getters
    ////////////////////////////////////////////////////////////////////////////////////
    bool GraphicsContext::Initialized()
    {
        return s_Initialized;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Static methods
    ////////////////////////////////////////////////////////////////////////////////////
    void GraphicsContext::AttachWindow(void* nativeWindow)
    {
        s_ActiveWindow = nativeWindow;
    }

    void GraphicsContext::Init()
    {
        LU_ASSERT(s_ActiveWindow, "[GraphicsContext] No window has been attached.");

        s_GraphicsContext.Init(s_ActiveWindow);
        s_Initialized = true;
    }

    void GraphicsContext::Destroy()
    {
        s_GraphicsContext.Destroy();
        s_ActiveWindow = nullptr;
        s_Initialized = false;
    }

    ContextSelect<Info::g_RenderingAPI>::Type& GraphicsContext::GetInternalContext()
    {
        return s_GraphicsContext;
    }

}