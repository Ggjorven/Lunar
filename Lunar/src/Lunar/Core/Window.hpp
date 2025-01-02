#pragma once

#include <cstdint>
#include <string_view>

#include "Lunar/Memory/Arc.hpp"

#include "Lunar/Core/Events.hpp"

#include "Lunar/Maths/Structs.hpp"

namespace Lunar
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Specification
    ////////////////////////////////////////////////////////////////////////////////////
    using EventCallbackFn = std::function<void(Event e)>;

    struct WindowSpecification
    {
    public:
        enum class BufferMode : uint8_t { Single = 1, Double = 2, Triple = 3 };
    public:
        // Window
        std::string_view Title;
        uint32_t Width, Height;

        EventCallbackFn EventCallback;

        // Renderer
        bool VSync;
        BufferMode Buffers;
    };
    
    ////////////////////////////////////////////////////////////////////////////////////
    // Window
    ////////////////////////////////////////////////////////////////////////////////////
    class Window
	{
    public:
        // Constructors & Destructor
        Window() = default;
        virtual ~Window() = default;

        // Methods
        virtual void PollEvents() = 0;
        virtual void SwapBuffers() = 0;

        virtual void Close() = 0;

        // Getters
        virtual Vec2<uint32_t> GetSize() const = 0;
        virtual Vec2<float> GetPosition() const = 0;

        // Setters
        virtual void SetTitle(std::string_view title) = 0;
        virtual void SetVSync(bool vsync) = 0;

        // Additional getters
        virtual bool IsVSync() const = 0;
        virtual bool IsOpen() const = 0;
        virtual bool IsMinimized() const = 0;

        virtual void* GetNativeWindow() = 0;
        virtual WindowSpecification& GetSpecification() = 0;

        // Static methods
        static Arc<Window> Create(const WindowSpecification& windowSpecs);
	};

}