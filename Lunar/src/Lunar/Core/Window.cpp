#include "lupch.h"
#include "Window.hpp"

#include "Lunar/IO/Print.hpp"

#include "Lunar/Platform/Desktop/DesktopWindow.hpp"

namespace Lunar
{

    Arc<Window> Window::Create(const WindowSpecification& windowSpecs)
    {
        #if defined(LU_PLATFORM_DESKTOP)
            return Arc<DesktopWindow>::Create(windowSpecs);
        #endif

        return nullptr;
    }

}