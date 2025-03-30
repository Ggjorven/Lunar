#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Lunar::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// Buffer Spec
	////////////////////////////////////////////////////////////////////////////////////
	enum class VertexInputRate : uint8_t
	{
		Vertex = 0,
		Instance = 1 // GLSL: gl_InstanceID
	};

	enum class DataType : uint8_t
	{
		None = 0, 
		Float, Float2, Float3, Float4, 
		Int, Int2, Int3, Int4, 
		UInt, UInt2, UInt3, UInt4, 
		Bool, 
		Mat3, Mat4
	};
	size_t DataTypeSize(DataType type);

	struct BufferElement
	{
	public:
		std::string Name;
		uint32_t Location;
		DataType Type;
		size_t Size;
		size_t Offset;

	public:
		BufferElement() = default;
		BufferElement(DataType type, uint32_t location, const std::string& name);
		~BufferElement() = default;

		uint32_t GetComponentCount() const;
	};

	struct BufferLayout
	{
	public:
		BufferLayout() = default;
		BufferLayout(const std::initializer_list<BufferElement>& elements);									// VertexInputRate = VertexInputRate::Vertex
		BufferLayout(const std::initializer_list<BufferElement>& elements, size_t instanceBufferStride);	// VertexInputRate = VertexInputRate::Instance
		~BufferLayout() = default;

		inline size_t GetStride() const { return m_Stride; }
		inline VertexInputRate GetVertexInputRate() const { return m_VertexInputRate; }
		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		inline std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		inline std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		inline std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		inline std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		void CalculateOffsetsAndStride();

	private:
		std::vector<BufferElement> m_Elements = { };
		size_t m_Stride = 0;

		VertexInputRate m_VertexInputRate = VertexInputRate::Vertex;
	};

	enum class BufferMemoryUsage : uint8_t
	{
		Unknown = 0,
		GPU, // GPU Only
		CPU, // CPU Only
		CPUToGPU, // CPU to GPU
		GPUToCPU, // GPU to CPU
		CPUCopy // CPU copy operations
	};

	// A general purpose buffer specification, usable for all buffer types
	// Note: This is only used in the constructor and not retrievable afterwards.
	struct BufferSpecification
	{
	public:
		BufferMemoryUsage Usage = BufferMemoryUsage::GPU;
	};

}
