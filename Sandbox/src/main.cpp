///////////////////////////////////////////////////////////
// Build test file
///////////////////////////////////////////////////////////
#include "Lunar/Internal/Core/Events.hpp"
#include "Lunar/Internal/Core/Window.hpp"

#include "Lunar/Internal/Enum/Bitwise.hpp"
#include "Lunar/Internal/Enum/Fuse.hpp"
#include "Lunar/Internal/Enum/Name.hpp"
#include "Lunar/Internal/Enum/Utilities.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include "Lunar/Internal/Maths/Logarithm.hpp"
#include "Lunar/Internal/Maths/Structs.hpp"

#include "Lunar/Internal/Memory/Arc.hpp"
#include "Lunar/Internal/Memory/AutoRelease.hpp"
#include "Lunar/Internal/Memory/Box.hpp"
#include "Lunar/Internal/Memory/Rc.hpp"

#include "Lunar/Internal/Renderer/Renderer.hpp"

#include "Lunar/Internal/Utils/Hash.hpp"
#include "Lunar/Internal/Utils/Preprocessor.hpp"
#include "Lunar/Internal/Utils/Profiler.hpp"
#include "Lunar/Internal/Utils/Types.hpp"

#include "Lunar/Enum/Name.hpp"
#include "Lunar/Enum/Fuse.hpp"

#include "Lunar/Maths/Structs.hpp"

///////////////////////////////////////////////////////////
// Test Enum
///////////////////////////////////////////////////////////
enum class Test : uint8_t
{
    Hi = 0,
    Hello = 27,
    Hey,
};

///////////////////////////////////////////////////////////
// Test main
///////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // Enum Test
    constexpr const Test compValue = Test::Hello;
    volatile const Test runtimeValue = Test::Hey;

    constexpr std::string_view compValueStr = Lunar::Enum::Name(compValue);
    LU_LOG_TRACE("Compile time: {0}", compValueStr);
    LU_LOG_TRACE("Runtime: {0}", Lunar::Enum::Name(runtimeValue));

    // Types Test
	constexpr std::string_view typeName = Lunar::Internal::Types::ConstexprName<Test>::TypeName;
	LU_LOG_TRACE("Type name: {0}", typeName);

    // Window Test
    Lunar::Internal::Window window({
        .Title = "Window",

        .Width = 1280,
        .Height = 720,

        .EventCallback = [](Lunar::Internal::Event) {}
    });

    window.GetSpecification().EventCallback = [&](Lunar::Internal::Event e)
    {
        Lunar::Internal::EventHandler handler(e);
        handler.Handle<Lunar::Internal::WindowResizeEvent>([&](Lunar::Internal::WindowResizeEvent& wre) { window.Resize(wre.GetWidth(), wre.GetHeight()); });
        handler.Handle<Lunar::Internal::WindowCloseEvent>([&](Lunar::Internal::WindowCloseEvent&) { window.Close(); });
    };

	// Renderer Test
    {
        Lunar::Internal::CommandBuffer cmdBuf(window.GetRenderer().GetID());

        while (window.IsOpen())
        {
            window.PollEvents();
            window.GetRenderer().BeginFrame();

            window.GetRenderer().Begin(cmdBuf);

            // ...

            window.GetRenderer().End(cmdBuf);
            window.GetRenderer().Submit(cmdBuf, Lunar::Internal::ExecutionPolicy::InOrder);

            window.GetRenderer().EndFrame();
            window.GetRenderer().Present();
            window.SwapBuffers();
        }
    }

    return 0;
}