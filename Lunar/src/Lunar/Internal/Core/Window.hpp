#pragma once

#include "Lunar/Internal/Utils/Settings.hpp"
#include "Lunar/Internal/Platform/Desktop/DesktopWindow.hpp"

namespace Lunar::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// Selection
	////////////////////////////////////////////////////////////////////////////////////
	template<Info::Platform Platform>
	struct WindowSelect;

	template<> struct WindowSelect<Info::Platform::Windows>		{ using Type = typename DesktopWindow; };
	template<> struct WindowSelect<Info::Platform::Linux>		{ using Type = typename DesktopWindow; };
	template<> struct WindowSelect<Info::Platform::MacOS>		{ using Type = typename DesktopWindow; };

	using WindowType = typename WindowSelect<Info::g_Platform>::Type;

	////////////////////////////////////////////////////////////////////////////////////
	// Window
	////////////////////////////////////////////////////////////////////////////////////
	class Window // Note: This class acts as an interface between different platform implementations
	{
	public:
		// Constructors & Destructor
		inline Window(const WindowSpecification& specs) : m_Window(specs, this) {}
		~Window() = default;

		// Methods
		inline void PollEvents() { m_Window.PollEvents(); }
		inline void SwapBuffers() { m_Window.SwapBuffers(); }

		// Note: This is not resizing the window, it's resizing the drawing area (on the internal renderer).
		inline void Resize(uint32_t width, uint32_t height) { m_Window.Resize(width, height); }
		inline void Close() { m_Window.Close(); }

		// Getters
		inline Vec2<uint32_t> GetSize() const { return m_Window.GetSize(); }
		inline Vec2<float> GetPosition() const { return m_Window.GetPosition(); }

		// Setters
		inline void SetTitle(std::string_view title) { m_Window.SetTitle(title); }
		inline void SetVSync(bool vsync) { m_Window.SetVSync(vsync); }

		// Additional getters
		inline bool IsVSync() const { return m_Window.IsVSync(); }
		inline bool IsOpen() const { return m_Window.IsOpen(); }
		inline bool IsMinimized() const { return m_Window.IsMinimized(); }

		inline void* GetNativeWindow() { return m_Window.GetNativeWindow(); }
		inline WindowSpecification& GetSpecification() { return m_Window.GetSpecification(); }
		inline Renderer& GetRenderer() { return m_Window.GetRenderer(); }

	private:
		WindowType m_Window;
	};

}