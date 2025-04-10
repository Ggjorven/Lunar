#include "lupch.h"
#include "Window.hpp"

#include "Lunar/Internal/IO/Print.hpp"

namespace Lunar
{

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
	Window::Window(const WindowSpecification& specs)
	{
		Init(specs);
	}

	Window::~Window()
	{
		m_Renderer.Destroy();
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Init
	////////////////////////////////////////////////////////////////////////////////////
	void Window::Init(const WindowSpecification& specs)
	{
		m_Window.Init({
			.Title = specs.Title,
			.Width = specs.Width,
			.Height = specs.Height,
			
			.EventCallback = [this](Internal::Event e) { OnEvent(e); },
			
			.VSync = specs.VSync,
			.Buffers = Internal::WindowSpecification::BufferMode::Triple,
		});

		m_Renderer.Init(m_Window.GetRenderer().GetID());
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void Window::PollEvents()
	{
		m_Window.PollEvents();
	}

	void Window::SwapBuffers()
	{
		m_Window.SwapBuffers();
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Private methods
	////////////////////////////////////////////////////////////////////////////////////
	void Window::OnEvent(Internal::Event& e)
	{
		Internal::EventHandler handler(e);
		handler.Handle<Internal::WindowResizeEvent>([this](Internal::WindowResizeEvent& wre) { m_Window.Resize(wre.GetWidth(), wre.GetHeight()); });
		handler.Handle<Internal::WindowCloseEvent>([this](Internal::WindowCloseEvent&) { m_Window.Close(); });
	}

}