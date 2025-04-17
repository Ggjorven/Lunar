#pragma once

#include "Lunar/Internal/Renderer/Renderer.hpp"

#include "Lunar/Renderer/RendererSpec.hpp"
#include "Lunar/Renderer/Texture.hpp"

#include <cstdint>
#include <vector>

namespace Lunar
{

	class Window;
	class Renderpass2D;

	////////////////////////////////////////////////////////////////////////////////////
	// Renderer
	////////////////////////////////////////////////////////////////////////////////////
	class Renderer
	{
	public:
		// Main methods
		void BeginFrame();
		void EndFrame();

		// Getters
		inline RendererID GetID() const { return static_cast<RendererID>(m_Renderer->GetID()); }

	private:
		// Constructor & Destructor
		Renderer() = default;
		~Renderer() = default;

		// Init & Destroy
		void Init(const Internal::RendererID renderer);
		void Destroy();

		// Private methods
		void Resize(uint32_t width, uint32_t height);

		void AddPass(Renderpass2D* pass);
		void RemovePass(Renderpass2D* pass);

		// Static methods
		static Renderer& GetRenderer(const RendererID renderer);

	private:
		Internal::Renderer* m_Renderer = nullptr;

		std::vector<Renderpass2D*> m_2DPasses = { };

		friend class Window;
		friend class Renderpass2D;
	};

}