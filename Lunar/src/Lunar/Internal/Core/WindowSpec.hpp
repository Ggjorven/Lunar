#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

#include "Lunar/Internal/Core/Events.hpp"

namespace Lunar::Internal
{

    using EventCallbackFn = std::function<void(Event e)>;

    ////////////////////////////////////////////////////////////////////////////////////
    // WindowSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct WindowSpecification
    {
    public:
        enum class BufferMode : uint8_t { Single = 1, Double = 2, Triple = 3 };
    public:
        // Window
        std::string_view Title = {};
        uint32_t Width = 0, Height = 0;

        EventCallbackFn EventCallback = nullptr;

        // Renderer
        bool VSync = false;
        BufferMode Buffers = BufferMode::Triple;
    };

}