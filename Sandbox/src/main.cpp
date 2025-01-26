#include "Lunar/Core/Window.hpp"
#include "Lunar/Renderer/Renderer.hpp"

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
            m_Window->GetRenderer()->BeginFrame();

            // ...

            m_Window->GetRenderer()->EndFrame();
            m_Window->GetRenderer()->Present();
            m_Window->SwapBuffers();
        }
    }

private:
    void OnEvent(Event e) 
    { 
        EventHandler handler(e);
        handler.Handle<WindowResizeEvent>([this](WindowResizeEvent& e) 
            { 
                m_Window->Resize(e.GetWidth(), e.GetHeight()); 
            });
        handler.Handle<WindowCloseEvent>([this](WindowCloseEvent&) 
            { 
                m_Running = false; 
                m_Window->Close();
            });
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