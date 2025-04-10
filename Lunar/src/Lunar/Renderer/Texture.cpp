#include "lupch.h"
#include "Texture.hpp"

#include "Lunar/Internal/IO/Print.hpp"

namespace Lunar
{

	////////////////////////////////////////////////////////////////////////////////////
	// Destructor
	////////////////////////////////////////////////////////////////////////////////////
	Texture::~Texture()
	{
		m_Image.Destroy(m_RendererID);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Init
	////////////////////////////////////////////////////////////////////////////////////
	void Texture::Init(const RendererID renderer, uint32_t width, uint32_t height)
	{
		m_RendererID = renderer;

		m_Image.Init(renderer, {
			.Usage = Internal::ImageUsage::Colour | Internal::ImageUsage::Sampled,
			.Layout = Internal::ImageLayout::ShaderRead,
			.Format = Internal::ImageFormat::RGBA,

			.Width = width, .Height = height,

			.MipMaps = true,
		}, {
			.MagFilter = Internal::FilterMode::Linear,
			.MinFilter = Internal::FilterMode::Linear,
			.Address = Internal::AddressMode::Repeat,
			.Mipmaps = Internal::MipmapMode::Linear,
		});
	}

	void Texture::Init(const RendererID renderer, const std::filesystem::path& path)
	{
		m_RendererID = renderer;

		m_Image.Init(renderer, {
			.Usage = Internal::ImageUsage::Colour | Internal::ImageUsage::Sampled,
			.Layout = Internal::ImageLayout::ShaderRead,
			.Format = Internal::ImageFormat::RGBA,

			.Width = 0, .Height = 0,

			.MipMaps = true,
		}, {
			.MagFilter = Internal::FilterMode::Linear,
			.MinFilter = Internal::FilterMode::Linear,
			.Address = Internal::AddressMode::Repeat,
			.Mipmaps = Internal::MipmapMode::Linear,
		}, path);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void Texture::SetData(void* data, size_t size)
	{
		m_Image.SetData(m_RendererID, data, size);
	}

	void Texture::Resize(uint32_t width, uint32_t height)
	{
		m_Image.Resize(m_RendererID, width, height);
	}

}