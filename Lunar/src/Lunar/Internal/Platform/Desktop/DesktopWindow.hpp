#pragma once

#include "Lunar/Maths/Structs.hpp"

#include "Lunar/Internal/Core/WindowSpec.hpp"
#include "Lunar/Internal/Utils/Preprocessor.hpp"
#include "Lunar/Internal/Renderer/Renderer.hpp"

#include <GLFW/glfw3.h>

namespace Lunar::Internal
{

	class Window;
	class DesktopWindow;

#if defined(LU_PLATFORM_DESKTOP)
	////////////////////////////////////////////////////////////////////////////////////
	// DesktopWindow
	////////////////////////////////////////////////////////////////////////////////////
	class DesktopWindow
	{
	public:
		// Constructors & Destructor
		DesktopWindow(const WindowSpecification& specs, Window* instance);
		~DesktopWindow();

		// Methods
		void PollEvents();
		void SwapBuffers();

		void Resize(uint32_t width, uint32_t height);
		inline void Close() { m_Closed = true; }

		// Getters
		inline Vec2<uint32_t> GetSize() const { return { m_Specification.Width, m_Specification.Height }; }
		Vec2<float> GetPosition() const;

		// Setters
		void SetTitle(std::string_view title);
		void SetVSync(bool vsync);

		// Additional getters
		inline bool IsVSync() const { return m_Specification.VSync; }
		inline bool IsOpen() const { return !m_Closed; }
		inline bool IsMinimized() const { return ((m_Specification.Width == 0) || (m_Specification.Height == 0)); }

		inline void* GetNativeWindow() { return static_cast<void*>(m_Window); }
		inline WindowSpecification& GetSpecification() { return m_Specification; }
		inline Renderer& GetRenderer() { return m_Renderer; }

	private:
		WindowSpecification m_Specification;
		bool m_Closed = false;

		GLFWwindow* m_Window = nullptr;

		Renderer m_Renderer = {};
	};
#endif

}