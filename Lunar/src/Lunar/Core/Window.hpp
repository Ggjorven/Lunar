#pragma once

#include "Lunar/Internal/Core/Window.hpp"

#include "Lunar/Internal/Maths/Structs.hpp"

#include "Lunar/Renderer/Renderer.hpp"

namespace Lunar
{

	////////////////////////////////////////////////////////////////////////////////////
	// WindowSpecification
	////////////////////////////////////////////////////////////////////////////////////
	struct WindowSpecification
	{
	public:
		std::string_view Title = {};
		uint32_t Width = 0, Height = 0;

		bool VSync = false;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// Window
	////////////////////////////////////////////////////////////////////////////////////
	class Window
	{
	public:
		// Constructor & Destructor
		Window() = default;
		Window(const WindowSpecification& specs);
		~Window();

		// Init
		void Init(const WindowSpecification& specs);

		// Methods
		void PollEvents();
		void SwapBuffers();

		// Getters
		inline double GetTime() const { return m_Window.GetTime(); }
		inline bool IsOpen() const { return m_Window.IsOpen(); }
		inline Vec2<uint32_t> GetSize() const { return m_Window.GetSize(); }

		inline Renderer& GetRenderer() { return m_Renderer; }

	private:
		// Private methods
		void OnEvent(Internal::Event& e);

	private:
		Internal::Window m_Window = {};

		Renderer m_Renderer = {};
	};

}