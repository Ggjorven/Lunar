#include "lupch.h"
#include "GraphicsContext.hpp"

#include "Lunar/IO/Print.hpp"

#include "Lunar/Memory/Box.hpp"

namespace Lunar
{

    namespace
    {
        static std::vector<void*> s_AttachedWindows = { };
        static Box<GraphicsContext> s_GraphicsContext = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Getters
    ////////////////////////////////////////////////////////////////////////////////////
    bool GraphicsContext::Initialized()
    {
        return (s_GraphicsContext != nullptr);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Static methods
    ////////////////////////////////////////////////////////////////////////////////////
    void GraphicsContext::AttachWindow(void* nativeWindow)
    {
        s_AttachedWindows.push_back(nativeWindow);

        #if defined(LU_PLATFORM_DESKTOP)

        #endif
    }

    void GraphicsContext::Init(uint32_t width, uint32_t height, bool vsync, uint8_t framesInFlight)
    {
        LU_ASSERT(!s_AttachedWindows.empty(), "[GraphicsContext] No window has been attached.");
        
        #if defined(LU_PLATFORM_DESKTOP)
        #endif
    }

    void GraphicsContext::Destroy()
    {
        s_GraphicsContext.Reset();
    }

}