#pragma once

#include "Lunar/Renderer/RendererSpec.hpp"

#include "Lunar/Internal/Renderer/Image.hpp"

#include <cstdint>
#include <filesystem>

namespace Lunar
{

	class Renderer;

	////////////////////////////////////////////////////////////////////////////////////
	// Texture
	////////////////////////////////////////////////////////////////////////////////////
	class Texture
	{
	public:
		// Constructors & Destructor
		Texture() = default;
		Texture(const RendererID renderer, uint32_t width, uint32_t height) { Init(renderer, width, height); }
		Texture(const RendererID renderer, const std::filesystem::path& path) { Init(renderer, path); }
		~Texture();

		// Init
		void Init(const RendererID renderer, uint32_t width, uint32_t height);
		void Init(const RendererID renderer, const std::filesystem::path& path);

		// Methods
		void SetData(void* data, size_t size);

		void Resize(uint32_t width, uint32_t height);

	private:
		RendererID m_RendererID = 0;
		Internal::Image m_Image = {};

		friend class Renderer;
	};

}