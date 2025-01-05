#include "Lunar/Core/Window.hpp"

using namespace Lunar;
class Application
{
public:
    Application()
    {
        m_Window = Window::Create({ 
            .Title = "Window Title",
            .Width = 1280,
            .Height = 720,

            .EventCallback = [this](Event e) { OnEvent(e); },

            .VSync = false,
            .Buffers = WindowSpecification::BufferMode::Triple,
        });
    }
    ~Application() 
    { 
        m_Window->Close();
    }

    void Run()
    {
        while (m_Running)
        {
            m_Window->PollEvents();

            // ...

            m_Window->SwapBuffers();
        }
    }

private:
    void OnEvent(Event e) 
    { 
        EventHandler handler(e);
        handler.Handle<WindowCloseEvent>([this](WindowCloseEvent& e) { m_Running = false; });
    }

private:
    bool m_Running = true;
    Arc<Window> m_Window = nullptr;
};

int main(int argc, char* argv[])
{
    Application app;
    app.Run();

    return 0;
}