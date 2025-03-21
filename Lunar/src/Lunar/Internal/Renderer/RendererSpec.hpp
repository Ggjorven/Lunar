#pragma once

#include <cstdint>
#include <functional>

#include "Lunar/Internal/Core/WindowSpec.hpp"
#include "Lunar/Internal/Enum/Bitwise.hpp"

namespace Lunar::Internal
{

    class Window;

    ////////////////////////////////////////////////////////////////////////////////////
    // Configurations
    ////////////////////////////////////////////////////////////////////////////////////
    enum class ExecutionPolicy : uint8_t
    {
        InOrder = 1 << 0,           // Execute commands sequentially, submits to waited on by next (WaitForPrevious) commandBuffer
        Parallel = 1 << 1,          // Execute commands in parallel but synchronized by the frame

        WaitForPrevious = 1 << 2,   // Wait for the completion of the previous (InOrder) command buffer
    };
    LU_ENABLE_BITWISE(ExecutionPolicy)

    using FreeFn = std::function<void()>;

    ////////////////////////////////////////////////////////////////////////////////////
    // RendererSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct RendererSpecification
    {
    public:
        Window* WindowRef = nullptr;

        uint32_t Width = 0, Height = 0;

        WindowSpecification::BufferMode Buffers = WindowSpecification::BufferMode::Triple;
        bool VSync = true;
    };

    using RendererID = uint8_t;

}