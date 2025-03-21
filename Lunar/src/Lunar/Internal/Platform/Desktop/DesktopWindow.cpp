#include "lupch.h"
#include "DesktopWindow.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include "Lunar/Internal/Utils/Profiler.hpp"

#include "Lunar/Internal/Renderer/GraphicsContext.hpp"

namespace Lunar::Internal
{

#if defined(LU_PLATFORM_DESKTOP)
    namespace
    {
        static uint8_t s_GLFWInstances = 0;

        static void GLFWErrorCallBack(int errorCode, const char* description)
        {
            LU_LOG_ERROR("[GLFW]: ({0}), {1}", errorCode, description);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructors & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    DesktopWindow::DesktopWindow(const WindowSpecification& specs, Window* instance)
        : m_Specification(specs)
    {
        LU_ASSERT(!specs.Title.empty(), "[DesktopWindow] No title passed in.");
        LU_ASSERT(((specs.Width != 0) && (specs.Height != 0)), "[DesktopWindow] Invalid width & height passed in.");
        LU_ASSERT(static_cast<bool>(specs.EventCallback), "[DesktopWindow] No event callback passed in.");

        // Initialize windowing library
        if (s_GLFWInstances == 0)
        {
            int result = glfwInit();
            LU_ASSERT((result), "[DesktopWindow] Failed to initialize windowing library.");

            s_GLFWInstances++;
            glfwSetErrorCallback(GLFWErrorCallBack);
        }

        // Create the window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_Window = glfwCreateWindow(static_cast<int>(specs.Width), static_cast<int>(specs.Height), specs.Title.data(), nullptr, nullptr);
        LU_ASSERT(m_Window, "[DesktopWindow] Failed to create a window.");

        // Initialize context
        GraphicsContext::AttachWindow(static_cast<void*>(m_Window));
        if (!GraphicsContext::Initialized())
            GraphicsContext::Init();

        // Making sure we can access the data in the callbacks
        glfwSetWindowUserPointer(m_Window, (void*)&m_Specification);

        // Setting the callbacks
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
            {
                WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);
                data.Width = width;
                data.Height = height;

                WindowResizeEvent event = WindowResizeEvent(width, height);
                data.EventCallback(event);
            });
        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
            {
                WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

                WindowCloseEvent event = WindowCloseEvent();
                data.EventCallback(event);
            });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int, int action, int)
            {
                WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

                switch (action)
                {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event = KeyPressedEvent(key, 0);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event = KeyReleasedEvent(key);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event = KeyPressedEvent(key, 1);
                    data.EventCallback(event);
                    break;
                }
                }
            });
        glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
            {
                WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

                KeyTypedEvent event = KeyTypedEvent(keycode);
                data.EventCallback(event);
            });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int)
            {
                WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

                switch (action) 
                {
                case GLFW_PRESS: 
                {
                    MouseButtonPressedEvent event = MouseButtonPressedEvent(button);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE: 
                {
                    MouseButtonReleasedEvent event = MouseButtonReleasedEvent(button);
                    data.EventCallback(event);
                    break;
                }
                }
            });
        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
            {
                WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

                MouseScrolledEvent event = MouseScrolledEvent((float)xOffset, (float)yOffset);
                data.EventCallback(event);
            });
        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
            {
                WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

                MouseMovedEvent event = MouseMovedEvent((float)xPos, (float)yPos);
                data.EventCallback(event);
            });

        m_Renderer.Init({
            .WindowRef = instance,

            .Width = m_Specification.Width,
            .Height = m_Specification.Height,

            .Buffers = m_Specification.Buffers,
            .VSync = m_Specification.VSync,
        });
        m_Renderer.Recreate(m_Specification.Width, m_Specification.Height, m_Specification.VSync);
    }

    DesktopWindow::~DesktopWindow()
    {
        m_Closed = true;

        m_Renderer.Destroy();

        bool destroy = (--s_GLFWInstances == 0);
        if (destroy)
        {
            GraphicsContext::Destroy();
        }

        glfwDestroyWindow(m_Window);
        m_Window = nullptr;

        if (destroy)
        {
            glfwTerminate();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void DesktopWindow::PollEvents()
    {
        LU_MARK_FRAME();
        LU_PROFILE("DesktopWindow::PollEvents");
        glfwPollEvents();
    }

    void DesktopWindow::SwapBuffers()
    {
        LU_PROFILE("DesktopWindow::SwapBuffers");
        m_Renderer.FreeQueue();
    }

    void DesktopWindow::Resize(uint32_t width, uint32_t height)
    {
        m_Renderer.Recreate(width, height, m_Specification.VSync);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Getters
    ////////////////////////////////////////////////////////////////////////////////////
    Vec2<float> DesktopWindow::GetPosition() const
    {
        int xPos = 0, yPos = 0;
        glfwGetWindowPos(m_Window, &xPos, &yPos);

        return { static_cast<float>(xPos), static_cast<float>(yPos) };
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Setters
    ////////////////////////////////////////////////////////////////////////////////////
    void DesktopWindow::SetTitle(std::string_view title)
    {
        m_Specification.Title = title;
        glfwSetWindowTitle(m_Window, m_Specification.Title.data());
    }

    void DesktopWindow::SetVSync(bool vsync)
    {
        m_Specification.VSync = vsync;
        m_Renderer.Recreate(m_Specification.Width, m_Specification.Height, vsync);
    }
#endif

}