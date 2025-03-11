#include "lupch.h"
#include "Events.hpp"

#include "Lunar/Internal/IO/Print.hpp"

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Window events
    ////////////////////////////////////////////////////////////////////////////////////
    WindowResizeEvent::WindowResizeEvent(uint32_t width, uint32_t height) 
        : m_Width(width), m_Height(height)
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Key events
    ////////////////////////////////////////////////////////////////////////////////////
    KeyPressedEvent::KeyPressedEvent(int keycode, int repeatCount) 
        : m_KeyCode((Key)keycode), m_RepeatCount(repeatCount)
    {
    }

    KeyReleasedEvent::KeyReleasedEvent(int keycode)
        : m_KeyCode((Key)keycode)
    {
    }

    KeyTypedEvent::KeyTypedEvent(int keycode)
        : m_KeyCode((Key)keycode)
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Mouse events
    ////////////////////////////////////////////////////////////////////////////////////
    MouseMovedEvent::MouseMovedEvent(float x, float y) 
        : m_MouseX(x), m_MouseY(y)
    {
    }

    MouseScrolledEvent::MouseScrolledEvent(float xOffset, float yOffset) 
        : m_XOffset(xOffset), m_YOffset(yOffset)
    {
    }

    MouseButtonPressedEvent::MouseButtonPressedEvent(int button) 
        : m_Button((MouseButton)button)
    {
    }

    MouseButtonReleasedEvent::MouseButtonReleasedEvent(int button)
        : m_Button((MouseButton)button)
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Event
    ////////////////////////////////////////////////////////////////////////////////////
    EventHandler::EventHandler(Event e) 
        : m_Event(e)
    {
    }

}