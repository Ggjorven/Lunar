#include "SandboxApp.hpp"

////////////////////////////////////////////////////////////////////////////////////
// Constructor & Destructor
////////////////////////////////////////////////////////////////////////////////////
SandboxApp::SandboxApp()
{
	m_Window.Init({
		.Title = "SandboxApp",
		.Width = 1280, .Height = 720,

		.VSync = true,
	});
	m_Window.SetEventCallback([this](Event e) { OnEvent(e); });

	m_ScenePass.Init(m_Window.GetRenderer().GetID(), LoadOperation::Clear);
	m_UIPass.Init(m_Window.GetRenderer().GetID(), LoadOperation::Load);

	m_ScenePass.Set2DCamera(Mat4(1.0f), Mat4(1.0f));
	m_UIPass.Set2DCamera(Mat4(1.0f), Mat4(1.0f));
}

SandboxApp::~SandboxApp()
{
}

////////////////////////////////////////////////////////////////////////////////////
// Methods
////////////////////////////////////////////////////////////////////////////////////
void SandboxApp::Update()
{
	m_Window.PollEvents();
}

void SandboxApp::Render()
{
	m_Window.GetRenderer().BeginFrame();

	m_ScenePass.Begin();
	m_ScenePass.End();

	m_UIPass.Begin();
	m_UIPass.End();

	m_Window.GetRenderer().EndFrame();

	m_Window.SwapBuffers();
}

////////////////////////////////////////////////////////////////////////////////////
// Private methods
////////////////////////////////////////////////////////////////////////////////////
void SandboxApp::OnEvent(Event event)
{
	EventHandler handler(event);

	handler.Handle<WindowResizeEvent>([this](WindowResizeEvent& wre)
	{
		m_ScenePass.Resize(wre.GetWidth(), wre.GetHeight());
		m_UIPass.Resize(wre.GetWidth(), wre.GetHeight());
	});
	handler.Handle<WindowCloseEvent>([this](WindowCloseEvent&)
	{
		m_Running = false;
	});
}
