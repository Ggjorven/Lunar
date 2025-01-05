#include "lupch.h"
#include "GraphicsContext.hpp"

#include "Lunar/IO/Print.hpp"

#include "Lunar/Memory/Box.hpp"

#if defined(LU_PLATFORM_DESKTOP)
#include "Lunar/API/Vulkan/VulkanContext.hpp"
#endif

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
    }

    void GraphicsContext::Init()
    {
        LU_ASSERT(!s_AttachedWindows.empty(), "[GraphicsContext] No window has been attached.");
        
        #if defined(LU_PLATFORM_DESKTOP)
        s_GraphicsContext = Box<VulkanContext>::Create(s_AttachedWindows);
        #endif
    }

    void GraphicsContext::Destroy()
    {
        s_GraphicsContext.Reset();
    }

}