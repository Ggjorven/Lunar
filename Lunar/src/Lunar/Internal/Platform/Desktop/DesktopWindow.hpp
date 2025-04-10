#pragma once

#include "Lunar/Maths/Structs.hpp"

#include "Lunar/Internal/Core/WindowSpec.hpp"
#include "Lunar/Internal/Utils/Preprocessor.hpp"
#include "Lunar/Internal/Renderer/Renderer.hpp"

#if defined(LU_PLATFORM_WINDOWS)
	#include <GLFW/glfw3.h>
#endif

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
		DesktopWindow() = default;
		~DesktopWindow() = default;

		// Init & Destroy
		void Init(const WindowSpecification& specs, Window* instance);
		void Destroy();

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
		double GetTime() const;
		inline bool IsVSync() const { return m_Specification.VSync; }
		inline bool IsOpen() const { return !m_Closed; }
		inline bool IsMinimized() const { return ((m_Specification.Width == 0) || (m_Specification.Height == 0)); }

		inline void* GetNativeWindow() { return static_cast<void*>(m_Window); }
		inline WindowSpecification& GetSpecification() { return m_Specification; }
		inline Renderer& GetRenderer() { return m_Renderer; }

	private:
		WindowSpecification m_Specification;
		
		GLFWwindow* m_Window = nullptr;
		
		bool m_Closed = false;
		
		Renderer m_Renderer = {};
	};
#endif

}