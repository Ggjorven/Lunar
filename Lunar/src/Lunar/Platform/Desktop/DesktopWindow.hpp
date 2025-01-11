#pragma once

#if defined(LU_PLATFORM_DESKTOP)
#include "Lunar/Core/Window.hpp"

#include "Lunar/Renderer/Renderer.hpp"

#include <GLFW/glfw3.h>

namespace Lunar
{

	class DesktopWindow : public Window
	{
    public:
        // Constructors & Destructor
        DesktopWindow(const WindowSpecification& specs);
        ~DesktopWindow();

        // Methods
        void PollEvents() override;
        void SwapBuffers() override;

        inline void Close() override { m_Closed = true; }

        // Getters
        inline Vec2<uint32_t> GetSize() const override { return { m_Specification.Width, m_Specification.Height }; }
        Vec2<float> GetPosition() const override;

        // Setters
        void SetTitle(std::string_view title) override;
        void SetVSync(bool vsync) override;

        // Additional getters
        inline bool IsVSync() const override { return m_Specification.VSync; }
        inline bool IsOpen() const override { return !m_Closed; }
        inline bool IsMinimized() const override { return ((m_Specification.Width == 0) || (m_Specification.Height == 0)); }

        inline void* GetNativeWindow() override { return static_cast<void*>(m_Window); }
        inline WindowSpecification& GetSpecification() override { return m_Specification; }
        inline Arc<Renderer> GetRenderer() override { return m_Renderer; }

	private:
        WindowSpecification m_Specification;
        bool m_Closed = false;

        GLFWwindow* m_Window = nullptr;

        Arc<Renderer> m_Renderer = nullptr;
	};

}
#endif