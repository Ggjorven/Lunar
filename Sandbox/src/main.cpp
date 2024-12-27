#include <iostream>

#include "Lunar/Core/Events.hpp"
#include "Lunar/Core/Window.hpp"

#include "Lunar/IO/Print.hpp"

#include "Lunar/Memory/Rc.hpp"
#include "Lunar/Memory/Arc.hpp"

#include "Lunar/Memory/Box.hpp"
#include "Lunar/Memory/AutoRelease.hpp"

#include "Lunar/Maths/Structs.hpp"

#include "Lunar/Utils/Profiler.hpp"

using namespace Lunar;

int main(int argc, char* argv[])
{
    Arc<Window> w = Window::Create({ 
        .Title = "Custom Window",
        .Width = 1280,
        .Height = 720,

        .EventCallback = [](Event e) {},

        .VSync = false,
        .Buffers = WindowSpecification::BufferMode::Triple
    });

    LU_LOG_TRACE("HI {0}", 1);
    LU_LOG_INFO("HI {0}", 1);
    LU_LOG_WARN("HI {0}", 1);
    LU_LOG_ERROR("HI {0}", 1);
    LU_LOG_FATAL("HI {0}", 1);

    LU_VERIFY(false, "Message");
    LU_ASSERT(false, "Message");

    return 0;
}