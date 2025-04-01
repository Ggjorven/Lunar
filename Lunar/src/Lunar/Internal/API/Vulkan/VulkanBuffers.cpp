#include "lupch.h"
#include "VulkanBuffers.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include "Lunar/Internal/Renderer/Renderer.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"

namespace Lunar::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// Init & Destroy
	////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////
	// Convert functions
	////////////////////////////////////////////////////////////////////////////////////
	VertexInputRate VkVertexInputRateToVertexInputRate(VkVertexInputRate input)
	{
		switch (input)
		{
		case VK_VERTEX_INPUT_RATE_VERTEX:			return VertexInputRate::Vertex;
		case VK_VERTEX_INPUT_RATE_INSTANCE:			return VertexInputRate::Instance;

		default:
			LU_ASSERT(false, "[VkBuffer] Invalid vertex input rate passed in.");
			break;
		}

		return VertexInputRate::Vertex;
	}

	VkVertexInputRate VertexInputRateToVkVertexInputRate(VertexInputRate input)
	{
		switch (input)
		{
		case VertexInputRate::Vertex:			return VK_VERTEX_INPUT_RATE_VERTEX;
		case VertexInputRate::Instance:			return VK_VERTEX_INPUT_RATE_INSTANCE;

		default:
			LU_ASSERT(false, "[VkBuffer] Invalid vertex input rate passed in.");
			break;
		}

		return VK_VERTEX_INPUT_RATE_VERTEX;
	}

	DataType VkFormatToDataType(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_R32_SFLOAT:					return DataType::Float;
		case VK_FORMAT_R32G32_SFLOAT:				return DataType::Float2;
		case VK_FORMAT_R32G32B32_SFLOAT:			return DataType::Float3; // Or Mat3
		case VK_FORMAT_R32G32B32A32_SFLOAT:			return DataType::Float4; // Or Mat4
		case VK_FORMAT_R32_SINT:					return DataType::Int;
		case VK_FORMAT_R32G32_SINT:					return DataType::Int2;
		case VK_FORMAT_R32G32B32_SINT:				return DataType::Int3;
		case VK_FORMAT_R32G32B32A32_SINT:			return DataType::Int4;
		case VK_FORMAT_R32_UINT:					return DataType::UInt;
		case VK_FORMAT_R32G32_UINT:					return DataType::UInt2;
		case VK_FORMAT_R32G32B32_UINT:				return DataType::UInt3;
		case VK_FORMAT_R32G32B32A32_UINT:			return DataType::UInt4;
		case VK_FORMAT_R8_UINT:						return DataType::Bool;

		default:
			LU_ASSERT(false, "[VkBuffer] Invalid format passed in.");
			break;
		}

		return DataType::None;
	}

	VkFormat DataTypeToVkFormat(DataType dataType)
	{
		switch (dataType)
		{
		case DataType::Float:						return VK_FORMAT_R32_SFLOAT;
		case DataType::Float2:						return VK_FORMAT_R32G32_SFLOAT;
		case DataType::Float3:						return VK_FORMAT_R32G32B32_SFLOAT;
		case DataType::Float4:						return VK_FORMAT_R32G32B32A32_SFLOAT;
		case DataType::Int:							return VK_FORMAT_R32_SINT;
		case DataType::Int2:						return VK_FORMAT_R32G32_SINT;
		case DataType::Int3:						return VK_FORMAT_R32G32B32_SINT;
		case DataType::Int4:						return VK_FORMAT_R32G32B32A32_SINT;
		case DataType::UInt:						return VK_FORMAT_R32_UINT;
		case DataType::UInt2:						return VK_FORMAT_R32G32_UINT;
		case DataType::UInt3:						return VK_FORMAT_R32G32B32_UINT;
		case DataType::UInt4:						return VK_FORMAT_R32G32B32A32_UINT;
		case DataType::Bool:						return VK_FORMAT_R8_UINT;
		case DataType::Mat3:						return VK_FORMAT_R32G32B32_SFLOAT;		// Assuming Mat3 is represented as 3 vec3s
		case DataType::Mat4:						return VK_FORMAT_R32G32B32A32_SFLOAT;	// Assuming Mat4 is represented as 4 vec4s

		default:
			LU_ASSERT(false, "[VkBuffer] Invalid DataType passed in.");
			break;
		}

		return VK_FORMAT_UNDEFINED;
	}

}