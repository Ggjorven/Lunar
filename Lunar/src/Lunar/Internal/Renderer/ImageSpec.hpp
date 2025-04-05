#pragma once

#include "Lunar/Internal/Enum/Bitwise.hpp"

#include <cstdint>

namespace Lunar::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// Image specs
	////////////////////////////////////////////////////////////////////////////////////
	enum class ImageUsage : uint32_t
	{
		None = 0,
		TransferSrc = 1 << 0,   // May be added by default if image is loaded by another image for example
		TransferDst = 1 << 1,   // May be added by default if image is loaded by another image for example
		Sampled = 1 << 2,
		Storage = 1 << 3,
		Colour = 1 << 4,
		DepthStencil = 1 << 5,
		Transient = 1 << 6,
		Input = 1 << 7,
		VideoDecodeDstKHR = 1 << 8,
		VideoDecodeSrcKHR = 1 << 9,
		VideoDecodeDpbKHR = 1 << 10,
		FragmentDensityMapEXT = 1 << 11,
		FragmentShadingRateKHR = 1 << 12,
		HostTransferEXT = 1 << 13,
		VideoEncodeDstKHR = 1 << 14,
		VideoEncodeSrcKHR = 1 << 15,
		VideoEncodeDpbKHR = 1 << 16,
		FeedbackLoopEXT = 1 << 17,
		InvocationMaskHuawei = 1 << 18,
		SampleWeightQCOM = 1 << 19,
		SampleBlockMatchQCOM = 1 << 20,
		ShadingRateImageNV = FragmentShadingRateKHR,
	};
	LU_ENABLE_BITWISE(ImageUsage)

	enum class ImageLayout : uint32_t
	{
		Undefined = 0,
		General,
		Colour,
		DepthStencil,
		DepthStencilRead,
		ShaderRead,
		TransferSrc,
		TransferDst,
		PreInitialized,
		DepthReadStencil,
		DepthReadStencilRead,
		Depth,
		DepthRead,
		Stencil,
		StencilRead,
		Read,
		Attachment,
		PresentSrcKHR,
		VideoDecodeDstKHR,
		VideoDecodeSrcKHR,
		VideoDecodeDpbKHR,
		SharedPresentKHR,
		FragmentDensityMapEXT,
		FragmentShadingRateKHR,
		RenderingLocalReadKHR,
		VideoEncodeDstKHR,
		VideoEncodeSrcKHR,
		VideoEncodeDpbKHR,
		FeedbackLoopEXT,

		DepthReadStencilKHR = DepthReadStencil,
		DepthStencilReadKHR = DepthStencilRead,
		ShadingRateNV = FragmentShadingRateKHR,
		DepthKHR = Depth,
		DepthReadKHR = DepthRead,
		StencilKHR = Stencil,
		StencilReadKHR = StencilRead,
		ReadKHR = Read,
		AttachmentKHR = Attachment
	};
	enum class ImageFormat : uint8_t // Only default formats since there are hundreds of formats
	{
		Undefined = 0,
		RGBA,
		BGRA,
		sRGB,
		Depth32SFloat,
		Depth32SFloatS8,
		Depth24UnormS8
	};

	struct ImageSpecification
	{
	public:
		ImageUsage Usage = ImageUsage::Sampled;
		ImageLayout Layout = ImageLayout::ShaderRead;
		ImageFormat Format = ImageFormat::RGBA;

		uint32_t Width = 0;
		uint32_t Height = 0;

		bool MipMaps = true;
	};

	///////////////////////////////////////////////////////////
	// Sampler specs
	///////////////////////////////////////////////////////////
	enum class FilterMode : uint8_t
	{
		Nearest = 0,
		Linear,
		CubicEXT,
		CubicIMG = CubicEXT,
	};

	enum class AddressMode : uint8_t
	{
		Repeat = 0,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder,
		MirrorClampToEdge,
		MirrorClampToEdgeKHR = MirrorClampToEdge,
	};

	enum class MipmapMode : uint8_t
	{
		Nearest = 0,
		Linear
	};

	struct SamplerSpecification
	{
	public:
		FilterMode MagFilter = FilterMode::Linear;
		FilterMode MinFilter = FilterMode::Linear;
		AddressMode Address = AddressMode::Repeat; // For U, V & W
		MipmapMode Mipmaps = MipmapMode::Linear;
	};

}