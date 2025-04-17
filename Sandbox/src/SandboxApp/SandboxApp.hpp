#pragma once

#include "Lunar/Core/Window.hpp"
#include "Lunar/Renderer/Renderpass.hpp"

using namespace Lunar;

class SandboxApp
{
public:
	// Constructor & Destructor
	SandboxApp();
	~SandboxApp();

	// Methods
	void Update();
	void Render();

	// Getters
	inline bool Running() const { return m_Running; }

private:
	// Private methods
	void OnEvent(Event event);

private:
	Window m_Window = {};
	bool m_Running = true;

	Renderpass2D m_UIPass = {};
	Renderpass2D m_ScenePass = {};
};