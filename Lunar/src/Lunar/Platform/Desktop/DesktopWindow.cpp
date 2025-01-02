#include "lupch.h"

#if defined(LU_PLATFORM_DESKTOP)
#include "DesktopWindow.hpp"

#include "Lunar/IO/Print.hpp"

#include "Lunar/Utils/Profiler.hpp"

namespace Lunar
{

    namespace
    {
        static uint32_t s_GLFWInstances = 0;

        static void GLFWErrorCallBack(int errorCode, const char* description)
        {
            LU_LOG_ERROR("[GLFW]: ({0}), {1}", errorCode, description);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructors & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    DesktopWindow::DesktopWindow(const WindowSpecification& specs) 
        : m_Specification(specs)
    {
        LU_ASSERT(!specs.Title.empty(), "[Window] No title passed in.");
        LU_ASSERT(((specs.Width != 0) && (specs.Height != 0)), "[Window] Invalid width & height passed in.");

        // Initialize windowing library
        if (s_GLFWInstances == 0) 
        {
            int result = glfwInit();
            LU_ASSERT((result), "[Window] Failed to initialize windowing library.");

            s_GLFWInstances++;
            glfwSetErrorCallback(GLFWErrorCallBack);
        }

        // Create the window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_Window = glfwCreateWindow((int)specs.Width, (int)specs.Height, specs.Title.data(), nullptr, nullptr);
        LU_ASSERT(m_Window, "[Window] Failed to create a window.");

        // TODO: Initialize GraphicsContext

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

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) 
            {
                WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

                switch (action) {
                    case GLFW_PRESS: {
                        KeyPressedEvent event = KeyPressedEvent(key, 0);
                        data.EventCallback(event);
                        break;
                    }
                    case GLFW_RELEASE: {
                        KeyReleasedEvent event = KeyReleasedEvent(key);
                        data.EventCallback(event);
                        break;
                    }
                    case GLFW_REPEAT: {
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

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) 
            {
                WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

                switch (action) {
                    case GLFW_PRESS: {
                        MouseButtonPressedEvent event = MouseButtonPressedEvent(button);
                        data.EventCallback(event);
                        break;
                    }
                    case GLFW_RELEASE: {
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
    }

    DesktopWindow::~DesktopWindow() 
    {
        m_Closed = true;

        //Renderer::Destroy();

        //GraphicsContext::Destroy();

        glfwDestroyWindow(m_Window);
        m_Window = nullptr;

        if (--s_GLFWInstances == 0)
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
        // Renderer::Recreate(m_Specification.Width, m_Specification.Height, vsync);
    }

}
#endif