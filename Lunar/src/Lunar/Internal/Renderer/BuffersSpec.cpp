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
		case DataType::Float:    return 4ull;
		case DataType::Float2:   return 4ull * 2;
		case DataType::Float3:   return 4ull * 3;
		case DataType::Float4:   return 4ull * 4;
		case DataType::Int:      return 4ull;
		case DataType::Int2:     return 4ull * 2;
		case DataType::Int3:     return 4ull * 3;
		case DataType::Int4:     return 4ull * 4;
		case DataType::UInt:     return 4ull;
		case DataType::UInt2:    return 4ull * 2;
		case DataType::UInt3:    return 4ull * 3;
		case DataType::UInt4:    return 4ull * 4;
		case DataType::Bool:     return 1ull;
		case DataType::Mat3:     return 4ull * 3 * 3;
		case DataType::Mat4:     return 4ull * 4 * 4;

		default:
			LU_ASSERT(false, "Unknown DataType!");
			break;
		}

		return 0;
	}

	BufferElement::BufferElement(DataType type, uint32_t location, const std::string& name, VertexInputRate inputRate)
		: Name(name), Location(location), Type(type), InputRate(inputRate), Size(DataTypeSize(type)), Offset(0)
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
		: m_Elements(elements)
	{
		CalculateOffsetsAndStrides();
	}

	size_t BufferLayout::GetStride(VertexInputRate inputRate) const
	{
		return (inputRate == VertexInputRate::Vertex ? m_VertexStride : m_InstanceStride);
	}

	void BufferLayout::CalculateOffsetsAndStrides()
	{
		size_t vertexOffset = 0;
		size_t instanceOffset = 0;

		for (auto& element : m_Elements)
		{
			if (element.InputRate == VertexInputRate::Vertex)
			{
				element.Offset = vertexOffset;
				vertexOffset += element.Size;
			}
			else if (element.InputRate == VertexInputRate::Instance)
			{
				element.Offset = instanceOffset;
				instanceOffset += element.Size;
			}
		}

		m_VertexStride = vertexOffset;
		m_InstanceStride = instanceOffset;
	}

}
