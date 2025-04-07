#include "lupch.h"
#include "VulkanBuffers.hpp"

#include "Lunar/Internal/IO/Print.hpp"
#include "Lunar/Internal/Utils/Profiler.hpp"

#include "Lunar/Internal/Renderer/Buffers.hpp"
#include "Lunar/Internal/Renderer/Renderer.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanAllocator.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"

namespace Lunar::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// Init & Destroy
	////////////////////////////////////////////////////////////////////////////////////
	void VulkanVertexBuffer::Init(const RendererID renderer, const BufferSpecification& specs, void* data, size_t size)
	{
		m_BufferSize = size;

		m_Allocation = VulkanAllocator::AllocateBuffer(renderer, m_BufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, (VmaMemoryUsage)specs.Usage, m_Buffer);

		// Only set data, if the data is valid
		if (data != nullptr)
			SetData(renderer, data, size, 0);
	}

	void VulkanVertexBuffer::Destroy(const RendererID renderer)
	{
		Renderer::GetRenderer(renderer).Free([rendererID = renderer, buffer = m_Buffer, allocation = m_Allocation]()
		{
			if (buffer != VK_NULL_HANDLE)
				VulkanAllocator::DestroyBuffer(rendererID, buffer, allocation);
		});
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void VulkanVertexBuffer::Bind(const RendererID renderer, CommandBuffer& cmdBuf) const
	{
		LU_PROFILE("VkVertexBuffer::Bind()");
		VulkanCommandBuffer& vkCmdBuf = cmdBuf.GetInternalCommandBuffer();

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(vkCmdBuf.GetVkCommandBuffer(VulkanRenderer::GetRenderer(renderer).GetVulkanSwapChain().GetCurrentFrame()), 0, 1, &m_Buffer, offsets);
	}

	void VulkanVertexBuffer::SetData(const RendererID renderer, void* data, size_t size, size_t offset)
	{
		// Ensure that the size + offset doesn't exceed bounds
		LU_VERIFY((size + offset <= m_BufferSize), "[VkVertexBuffer] Size and offset exceeds the buffer's bounds");

		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;
		stagingBufferAllocation = VulkanAllocator::AllocateBuffer(renderer, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

		void* mappedData = nullptr;
		VulkanAllocator::MapMemory(stagingBufferAllocation, mappedData);
		std::memcpy(mappedData, data, size);  // Copy only 'size' bytes
		VulkanAllocator::UnMapMemory(stagingBufferAllocation);

		// Copy data from the staging buffer to the vertex buffer at the specified offset
		VulkanAllocator::CopyBuffer(renderer, stagingBuffer, m_Buffer, size, offset);
		VulkanAllocator::DestroyBuffer(renderer, stagingBuffer, stagingBufferAllocation);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Static methods
	////////////////////////////////////////////////////////////////////////////////////
	void VulkanVertexBuffer::Bind(const RendererID renderer, CommandBuffer& commandBuffer, const std::vector<VertexBuffer*>& buffers)
	{
		LU_PROFILE("VkVertexBuffer::Bind(Buffers)");
		VulkanCommandBuffer& vkCmdBuf = commandBuffer.GetInternalCommandBuffer();

		std::vector<VkBuffer> vkBuffers;
		vkBuffers.reserve(buffers.size());

		std::vector<VkDeviceSize> offsets(buffers.size(), 0);

		for (auto& buffer : buffers)
		{
			VulkanVertexBuffer& vkVertexBuffer = buffer->GetInternalVertexBuffer();
			vkBuffers.push_back(vkVertexBuffer.m_Buffer);
		}

		vkCmdBindVertexBuffers(vkCmdBuf.GetVkCommandBuffer(VulkanRenderer::GetRenderer(renderer).GetVulkanSwapChain().GetCurrentFrame()), 0, static_cast<uint32_t>(vkBuffers.size()), vkBuffers.data(), offsets.data());
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Init & Destroy
	////////////////////////////////////////////////////////////////////////////////////
	void VulkanIndexBuffer::Init(const RendererID renderer, const BufferSpecification& specs, uint8_t* indices, uint32_t count)
	{
		m_Count = count;
		m_Type = Type::UInt8;

		VkDeviceSize bufferSize = sizeof(uint32_t) * count;
		m_Allocation = VulkanAllocator::AllocateBuffer(renderer, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, (VmaMemoryUsage)specs.Usage, m_Buffer);

		SetData(renderer, indices, count, 0);
	}

	void VulkanIndexBuffer::Init(const RendererID renderer, const BufferSpecification& specs, uint16_t* indices, uint32_t count)
	{
		m_Count = count;
		m_Type = Type::UInt16;

		VkDeviceSize bufferSize = sizeof(uint32_t) * count;
		m_Allocation = VulkanAllocator::AllocateBuffer(renderer, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, (VmaMemoryUsage)specs.Usage, m_Buffer);

		SetData(renderer, indices, count, 0);
	}

	void VulkanIndexBuffer::Init(const RendererID renderer, const BufferSpecification& specs, uint32_t* indices, uint32_t count)
	{
		m_Count = count;
		m_Type = Type::UInt32;

		VkDeviceSize bufferSize = sizeof(uint32_t) * count;
		m_Allocation = VulkanAllocator::AllocateBuffer(renderer, bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, (VmaMemoryUsage)specs.Usage, m_Buffer);

		SetData(renderer, indices, count, 0);
	}

	void VulkanIndexBuffer::Destroy(const RendererID renderer)
	{
		Renderer::GetRenderer(renderer).Free([renderer = renderer, buffer = m_Buffer, allocation = m_Allocation]()
		{
			if (buffer != VK_NULL_HANDLE)
				VulkanAllocator::DestroyBuffer(renderer, buffer, allocation);
		});
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void VulkanIndexBuffer::Bind(const RendererID renderer, CommandBuffer& cmdBuf) const
	{
		LU_PROFILE("VkIndexBuffer::Bind()");
		VulkanCommandBuffer& vkCmdBuf = cmdBuf.GetInternalCommandBuffer();

		auto indexType = (m_Type == Type::UInt8) ? VK_INDEX_TYPE_UINT8 : (m_Type == Type::UInt16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
		vkCmdBindIndexBuffer(vkCmdBuf.GetVkCommandBuffer(VulkanRenderer::GetRenderer(renderer).GetVulkanSwapChain().GetCurrentFrame()), m_Buffer, 0, indexType);
	}

	void VulkanIndexBuffer::SetData(const RendererID renderer, uint8_t* indices, uint32_t count, size_t countOffset)
	{
		// Ensure that the size + offset doesn't exceed bounds
		LU_VERIFY((count + countOffset <= m_Count), "[VkIndexBuffer] Count and countOffset exceeds the buffer's bounds");
		SetData(renderer, static_cast<void*>(indices), static_cast<size_t>((sizeof(uint8_t) * count)), static_cast<size_t>((sizeof(uint8_t) * countOffset)));
	}

	void VulkanIndexBuffer::SetData(const RendererID renderer, uint16_t* indices, uint32_t count, size_t countOffset)
	{
		// Ensure that the size + offset doesn't exceed bounds
		LU_VERIFY((count + countOffset <= m_Count), "[VkIndexBuffer] Count and countOffset exceeds the buffer's bounds");
		SetData(renderer, static_cast<void*>(indices), static_cast<size_t>((sizeof(uint16_t) * count)), static_cast<size_t>((sizeof(uint16_t) * countOffset)));
	}

	void VulkanIndexBuffer::SetData(const RendererID renderer, uint32_t* indices, uint32_t count, size_t countOffset)
	{
		// Ensure that the size + offset doesn't exceed bounds
		LU_VERIFY((count + countOffset <= m_Count), "[VkIndexBuffer] Count and countOffset exceeds the buffer's bounds");
		SetData(renderer, static_cast<void*>(indices), static_cast<size_t>((sizeof(uint32_t) * count)), static_cast<size_t>((sizeof(uint32_t) * countOffset)));
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Private methods
	////////////////////////////////////////////////////////////////////////////////////
	void VulkanIndexBuffer::SetData(const RendererID renderer, void* indices, size_t size, size_t offset)
	{
		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;
		stagingBufferAllocation = VulkanAllocator::AllocateBuffer(renderer, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer);

		void* mappedData = nullptr;
		VulkanAllocator::MapMemory(stagingBufferAllocation, mappedData);
		std::memcpy(mappedData, indices, size);  // Copy only 'size' bytes
		VulkanAllocator::UnMapMemory(stagingBufferAllocation);

		// Copy data from the staging buffer to the vertex buffer at the specified offset
		VulkanAllocator::CopyBuffer(renderer, stagingBuffer, m_Buffer, size, offset);
		VulkanAllocator::DestroyBuffer(renderer, stagingBuffer, stagingBufferAllocation);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Init & Destroy
	////////////////////////////////////////////////////////////////////////////////////
	void VulkanUniformBuffer::Init(const RendererID renderer, const BufferSpecification& specs, size_t dataSize)
	{
		m_Size = dataSize;

		#if defined(LU_CONFIG_DEBUG)
			if (specs.Usage == BufferMemoryUsage::GPU)
			{
				LU_LOG_WARN("[VkUniformBuffer] Creating a UniformBuffer solely on the GPU. This means SetData() cannot be used. Was this intented? If not, use: CPUToGPU.");
			}
		#endif

		const size_t framesInFlight = static_cast<size_t>(Renderer::GetRenderer(renderer).GetSpecification().Buffers);
		m_Buffers.resize(framesInFlight);
		m_Allocations.resize(framesInFlight);

		for (size_t i = 0; i < framesInFlight; i++)
			m_Allocations[i] = VulkanAllocator::AllocateBuffer(renderer, (VkDeviceSize)dataSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, (VmaMemoryUsage)specs.Usage, m_Buffers[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	}

	void VulkanUniformBuffer::Destroy(const RendererID renderer)
	{
		Renderer::GetRenderer(renderer).Free([renderer = renderer, buffers = m_Buffers, allocations = m_Allocations]()
		{
			for (size_t i = 0; i < buffers.size(); i++)
			{
				if (buffers[i] != VK_NULL_HANDLE)
					VulkanAllocator::DestroyBuffer(renderer, buffers[i], allocations[i]);
			}
		});
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void VulkanUniformBuffer::SetData(const RendererID renderer, void* data, size_t size, size_t offset)
	{
		LU_ASSERT((size + offset <= m_Size), "[VkUniformBuffer] Data exceeds buffer size.");

		const size_t framesInFlight = static_cast<size_t>(Renderer::GetRenderer(renderer).GetSpecification().Buffers);
		for (size_t i = 0; i < framesInFlight; i++)
		{
			void* mappedMemory = nullptr;
			VulkanAllocator::MapMemory(m_Allocations[i], mappedMemory);
			memcpy(static_cast<uint8_t*>(mappedMemory) + offset, data, size);
			VulkanAllocator::UnMapMemory(m_Allocations[i]);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Init & Destroy
	////////////////////////////////////////////////////////////////////////////////////
	void VulkanStorageBuffer::Init(const RendererID renderer, const BufferSpecification& specs, size_t dataSize)
	{
		m_Size = dataSize;

		#if defined(LU_CONFIG_DEBUG)
			if (specs.Usage == BufferMemoryUsage::GPU)
			{
				LU_LOG_WARN("[VkStorageBuffer] Creating a StorageBuffer solely on the GPU. This means SetData() cannot be used. Was this intented? If not, use: CPUToGPU.");
			}
		#endif

		const size_t framesInFlight = static_cast<size_t>(Renderer::GetRenderer(renderer).GetSpecification().Buffers);
		m_Buffers.resize(framesInFlight);
		m_Allocations.resize(framesInFlight);

		for (size_t i = 0; i < framesInFlight; i++)
			m_Allocations[i] = VulkanAllocator::AllocateBuffer(renderer, (VkDeviceSize)dataSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, (VmaMemoryUsage)specs.Usage, m_Buffers[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
	}

	void VulkanStorageBuffer::Destroy(const RendererID renderer)
	{
		Renderer::GetRenderer(renderer).Free([renderer = renderer, buffers = m_Buffers, allocations = m_Allocations]()
		{
			for (size_t i = 0; i < buffers.size(); i++)
			{
				if (buffers[i] != VK_NULL_HANDLE)
					VulkanAllocator::DestroyBuffer(renderer, buffers[i], allocations[i]);
			}
		});
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void VulkanStorageBuffer::SetData(const RendererID renderer, void* data, size_t size, size_t offset)
	{
		LU_ASSERT((size + offset <= m_Size), "[VkStorageBuffer] Data exceeds buffer size.");

		const size_t framesInFlight = static_cast<size_t>(Renderer::GetRenderer(renderer).GetSpecification().Buffers);
		for (size_t i = 0; i < framesInFlight; i++)
		{
			void* mappedMemory = nullptr;
			VulkanAllocator::MapMemory(m_Allocations[i], mappedMemory);
			memcpy(static_cast<uint8_t*>(mappedMemory) + offset, data, size);
			VulkanAllocator::UnMapMemory(m_Allocations[i]);
		}
	}

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