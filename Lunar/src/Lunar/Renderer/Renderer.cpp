#include "lupch.h"
#include "Renderer.hpp"

#include "Lunar/IO/Print.hpp"

#if defined(LU_PLATFORM_DESKTOP)
#include "Lunar/API/Vulkan/VulkanRenderer.hpp"
#endif

namespace Lunar
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Static methods
    ////////////////////////////////////////////////////////////////////////////////////
    Arc<Renderer> Renderer::Create(const RendererSpecification& specs)
    {
        #if defined(LU_PLATFORM_DESKTOP)
            return Arc<VulkanRenderer>::Create(specs);
        #else
            #error Renderer: Unsupported platform
            return nullptr;
        #endif
    }

}