#include "lupch.h"
#include "BuffersSpec.hpp"

#include "Lunar/Internal/IO/Print.hpp"

namespace Lunar::Internal
{
	////////////////////////////////////////////////////////////////////////////////////
	// Buffer Spec
	////////////////////////////////////////////////////////////////////////////////////
	size_t DataTypeSize(DataType type)
	{
		switch (type)
		{
		case DataType::Float:    return 4;
		case DataType::Float2:   return 4 * 2;
		case DataType::Float3:   return 4 * 3;
		case DataType::Float4:   return 4 * 4;
		case DataType::Int:      return 4;
		case DataType::Int2:     return 4 * 2;
		case DataType::Int3:     return 4 * 3;
		case DataType::Int4:     return 4 * 4;
		case DataType::UInt:     return 4;
		case DataType::UInt2:    return 4 * 2;
		case DataType::UInt3:    return 4 * 3;
		case DataType::UInt4:    return 4 * 4;
		case DataType::Bool:     return 1;
		case DataType::Mat3:     return 4 * 3 * 3;
		case DataType::Mat4:     return 4 * 4 * 4;

		default:
			LU_ASSERT(false, "Unknown DataType!");
			break;
		}

		return 0;
	}

	BufferElement::BufferElement(DataType type, uint32_t location, const std::string& name)
		: Name(name), Location(location), Type(type), Size(DataTypeSize(type)), Offset(0)
	{
	}

	uint32_t BufferElement::GetComponentCount() const
	{
		switch (Type)
		{
		case DataType::Float:   return 1;
		case DataType::Float2:  return 2;
		case DataType::Float3:  return 3;
		case DataType::Float4:  return 4;
		case DataType::Int:     return 1;
		case DataType::Int2:    return 2;
		case DataType::Int3:    return 3;
		case DataType::Int4:    return 4;
		case DataType::UInt:    return 1;
		case DataType::UInt2:   return 2;
		case DataType::UInt3:   return 3;
		case DataType::UInt4:   return 4;
		case DataType::Bool:    return 1;
		case DataType::Mat3:    return 3 * 3;
		case DataType::Mat4:    return 4 * 4;

		default:
			LU_ASSERT(false, "Unknown DataType!");
			break;
		}

		return 0;
	}

	BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements)
		: m_Elements(elements), m_VertexInputRate(VertexInputRate::Vertex)
	{
		CalculateOffsetsAndStride();
	}

	BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements, size_t instanceBufferStride)
		: m_Elements(elements), m_VertexInputRate(VertexInputRate::Vertex), m_Stride(instanceBufferStride)
	{
	}

	void BufferLayout::CalculateOffsetsAndStride()
	{
		size_t offset = 0;
		m_Stride = 0;
		for (auto& element : m_Elements)
		{
			element.Offset = offset;
			offset += element.Size;
			m_Stride += element.Size;
		}
	}

}
