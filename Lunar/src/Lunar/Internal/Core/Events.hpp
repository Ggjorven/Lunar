#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <variant>
#include <concepts>
#include <functional>
#include <type_traits>

#include "Lunar/Internal/Utils/Types.hpp"
#include "Lunar/Internal/Enum/Bitwise.hpp"

#include "Lunar/Internal/Core/Input/KeyCodes.hpp"
#include "Lunar/Internal/Core/Input/MouseCodes.hpp"

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Windows events
    ////////////////////////////////////////////////////////////////////////////////////
    class WindowResizeEvent
    {
    public:
        // Constructors & Destructor
        WindowResizeEvent(uint32_t width, uint32_t height);
        ~WindowResizeEvent() = default;

        // Getters
        inline uint32_t GetWidth() const { return m_Width; }
        inline uint32_t GetHeight() const { return m_Height; }

    private:
        uint32_t m_Width, m_Height;
    };

    class WindowCloseEvent
    {
    public:
        // Constructors & Destructor
        WindowCloseEvent() = default;
        ~WindowCloseEvent() = default;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Key events
    ////////////////////////////////////////////////////////////////////////////////////
    class KeyPressedEvent
    {
    public:
        // Constructors & Destructor
        KeyPressedEvent(int keycode, int repeatCount);
        ~KeyPressedEvent() = default;

        // Getters
        inline Key GetKeyCode() const { return m_KeyCode; }
        inline int GetRepeatCount() const { return m_RepeatCount; }

    private:
        Key m_KeyCode;
        int m_RepeatCount;
    };

    class KeyReleasedEvent
    {
    public:
        // Constructors & Destructor
        KeyReleasedEvent(int keycode);
        ~KeyReleasedEvent() = default;

        // Getters
        inline Key GetKeyCode() const { return m_KeyCode; }

    private:
        Key m_KeyCode;
    };

    class KeyTypedEvent
    {
    public:
        // Constructors & Destructor
        KeyTypedEvent(int keycode);
        ~KeyTypedEvent() = default;

        // Getters
        inline Key GetKeyCode() const { return m_KeyCode; }

    private:
        Key m_KeyCode;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Mouse events
    ////////////////////////////////////////////////////////////////////////////////////
    class MouseMovedEvent
    {
    public:
        // Constructors & Destructor
        MouseMovedEvent(float x, float y);
        ~MouseMovedEvent() = default;

        // Getters
        inline float GetX() const { return m_MouseX; }
        inline float GetY() const { return m_MouseY; }

    private:
        float m_MouseX, m_MouseY;
    };

    class MouseScrolledEvent
    {
    public:
        // Constructors & Destructor
        MouseScrolledEvent(float xOffset, float yOffset);
        ~MouseScrolledEvent() = default;

        // Getters
        inline float GetXOffset() const { return m_XOffset; }
        inline float GetYOffset() const { return m_YOffset; }

    private:
        float m_XOffset, m_YOffset;
    };

    class MouseButtonPressedEvent
    {
    public:
        // Constructors & Destructor
        MouseButtonPressedEvent(int button);
        ~MouseButtonPressedEvent() = default;

        // Getters
        inline MouseButton GetButtonCode() const { return m_Button; }

    private:
        MouseButton m_Button;
    };

    class MouseButtonReleasedEvent
    {
    public:
        // Constructors & Destructor
        MouseButtonReleasedEvent(int button);
        ~MouseButtonReleasedEvent() = default;

        // Getters
        inline MouseButton GetButtonCode() const { return m_Button; }

    private:
        MouseButton m_Button;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Event
    ////////////////////////////////////////////////////////////////////////////////////
    using Event = std::variant<
        WindowResizeEvent, WindowCloseEvent,
        KeyPressedEvent, KeyReleasedEvent, KeyTypedEvent,
        MouseMovedEvent, MouseScrolledEvent, MouseButtonPressedEvent, MouseButtonReleasedEvent
    >;

    class EventHandler
    {
    public:
        // Constructors & Destructor
        EventHandler(Event e);
        ~EventHandler() = default;

        // Methods
        template <typename TEvent, typename F>
        inline void Handle(const F&& func) requires (Types::TypeInVariant<TEvent, Event> && std::invocable<F, TEvent&>)
        {
            std::visit(
                [&](auto&& obj) 
                {
                    if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, TEvent>) 
                    {
                        func(obj);
                    }
                },
            m_Event);
        }

    private:
        Event m_Event;
    };

}
