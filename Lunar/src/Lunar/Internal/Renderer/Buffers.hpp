#pragma once

#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/Renderer/RendererSpec.hpp"
#include "Lunar/Internal/Renderer/BuffersSpec.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanBuffers.hpp"

#include <cstdint>
#include <string>

namespace Lunar::Internal
{

	class CommandBuffer;

	////////////////////////////////////////////////////////////////////////////////////
	// Selection // TODO: Implement DynamicUniformBuffer
	////////////////////////////////////////////////////////////////////////////////////
	template<Info::RenderingAPI API>
	struct VertexBufferSelect;
	template<Info::RenderingAPI API>
	struct IndexBufferSelect;
	template<Info::RenderingAPI API>
	struct UniformBufferSelect;
	//template<Info::RenderingAPI API>
	//struct DynamicUniformBufferSelect;
	template<Info::RenderingAPI API>
	struct StorageBufferSelect;

	template<> struct VertexBufferSelect<Info::RenderingAPI::Vulkan> { using Type = VulkanVertexBuffer; };
	template<> struct IndexBufferSelect<Info::RenderingAPI::Vulkan> { using Type = VulkanIndexBuffer; };
	template<> struct UniformBufferSelect<Info::RenderingAPI::Vulkan> { using Type = VulkanUniformBuffer; };
	//template<> struct DynamicUniformBufferSelect<Info::RenderingAPI::Vulkan> { using Type = VulkanDynamicUniformBuffer; };
	template<> struct StorageBufferSelect<Info::RenderingAPI::Vulkan> { using Type = VulkanStorageBuffer; };

	using VertexBufferType = typename VertexBufferSelect<Info::g_RenderingAPI>::Type;
	using IndexBufferType = typename IndexBufferSelect<Info::g_RenderingAPI>::Type;
	using UniformBufferType = typename UniformBufferSelect<Info::g_RenderingAPI>::Type;
	//using DynamicUniformBufferType = typename DynamicUniformBufferSelect<Info::g_RenderingAPI>::Type;
	using StorageBufferType = typename StorageBufferSelect<Info::g_RenderingAPI>::Type;

	////////////////////////////////////////////////////////////////////////////////////
	// VertexBuffer
	////////////////////////////////////////////////////////////////////////////////////
	class VertexBuffer
	{
	public:
		// Constructor & Destructor
		inline VertexBuffer(const RendererID renderer, const BufferSpecification& specs, void* data, size_t size) { m_VertexBuffer.Init(renderer, specs, data, size); }
		inline ~VertexBuffer() { m_VertexBuffer.Destroy(); }

		// Methods
		inline void Bind(CommandBuffer& cmdBuf) const { m_VertexBuffer.Bind(cmdBuf); }

		inline void SetData(void* data, size_t size, size_t offset = 0) { m_VertexBuffer.SetData(data, size, offset); }
		
		// Static methods
		inline static void Bind(const RendererID renderer, CommandBuffer& cmdBuf, const std::vector<VertexBuffer*>& buffers) { VertexBufferType::Bind(renderer,cmdBuf, buffers); }

		// Internal
		inline VertexBufferType& GetInternalVertexBuffer() { return m_VertexBuffer; }

	private:
		VertexBufferType m_VertexBuffer = {};
	};

	////////////////////////////////////////////////////////////////////////////////////
	// IndexBuffer
	////////////////////////////////////////////////////////////////////////////////////
	class IndexBuffer
	{
	public:
		// Constructor & Destructor
		inline IndexBuffer(const RendererID renderer, const BufferSpecification& specs, uint8_t* indices, uint32_t count) { m_IndexBuffer.Init(renderer, specs, indices, count); }
		inline IndexBuffer(const RendererID renderer, const BufferSpecification& specs, uint16_t* indices, uint32_t count) { m_IndexBuffer.Init(renderer, specs, indices, count); }
		inline IndexBuffer(const RendererID renderer, const BufferSpecification& specs, uint32_t* indices, uint32_t count) { m_IndexBuffer.Init(renderer, specs, indices, count); }
		inline ~IndexBuffer() { m_IndexBuffer.Destroy(); }

		// Methods
		inline void Bind(CommandBuffer& cmdBuf) const { m_IndexBuffer.Bind(cmdBuf); }

		// The offset is in count, so 1,2,3,4,5 instead of size(uint32_t)
		inline void SetData(uint8_t* indices, uint32_t count, size_t countOffset = 0) { m_IndexBuffer.SetData(indices, count, countOffset); }
		inline void SetData(uint16_t* indices, uint32_t count, size_t countOffset = 0) { m_IndexBuffer.SetData(indices, count, countOffset); }
		inline void SetData(uint32_t* indices, uint32_t count, size_t countOffset = 0) { m_IndexBuffer.SetData(indices, count, countOffset); }

		// Getters
		inline uint32_t GetCount() const { return m_IndexBuffer.GetCount(); }

		// Internal
		inline IndexBufferType& GetInternalIndexBuffer() { return m_IndexBuffer; }

	private:
		IndexBufferType m_IndexBuffer = {};
	};

	////////////////////////////////////////////////////////////////////////////////////
	// UniformBuffer
	////////////////////////////////////////////////////////////////////////////////////
	// Note: Needs to be created after the pipeline
	class UniformBuffer
	{
	public:
		// Constructor & Destructor
		inline UniformBuffer(const RendererID renderer, const BufferSpecification& specs, size_t dataSize) { m_UniformBuffer.Init(renderer, specs, dataSize); }
		inline ~UniformBuffer() { m_UniformBuffer.Destroy(); }

		// Methods
		inline void SetData(void* data, size_t size, size_t offset = 0) { m_UniformBuffer.SetData(data, size, offset); }

		// Getters
		virtual size_t GetSize() const { return m_UniformBuffer.GetSize(); }

		// Internal 
		inline UniformBufferType& GetInternalUniformBuffer() { return m_UniformBuffer; }

	private:
		UniformBufferType m_UniformBuffer = {};
	};

	////////////////////////////////////////////////////////////////////////////////////
	// DynamicUniformBuffer
	////////////////////////////////////////////////////////////////////////////////////
	/* // TODO: To be implemented. (Come up with a better design)
	class DynamicUniformBuffer : public RefCounted
	{
	public:
		DynamicUniformBuffer();
		~DynamicUniformBuffer();

		void SetData(void* data, size_t size);

		virtual void SetDataIndexed(uint32_t index, void* data, size_t size) = 0;
		virtual void UploadIndexedData() = 0;

		virtual uint32_t GetAmountOfElements() const = 0;
		virtual size_t GetAlignment() const = 0;

		//virtual void Upload(Ref<DescriptorSet> set, Descriptor element) = 0;
		//virtual void Upload(Ref<DescriptorSet> set, Descriptor element, size_t offset) = 0;

		// Returns underlying type pointer
		inline UniformBufferType* Src() { return m_Instance; }

		static Ref<DynamicUniformBuffer> Create(uint32_t elements, size_t sizeOfOneElement);
	};
	*/

	////////////////////////////////////////////////////////////////////////////////////
	// StorageBuffer
	////////////////////////////////////////////////////////////////////////////////////
	class StorageBuffer
	{
	public:
		// Constructor & Destructor
		inline StorageBuffer(const RendererID renderer, const BufferSpecification& specs, size_t dataSize) { m_StorageBuffer.Init(renderer, specs, dataSize); }
		inline ~StorageBuffer() { m_StorageBuffer.Destroy(); }

		// Methods
		inline void SetData(void* data, size_t size, size_t offset = 0) { m_StorageBuffer.SetData(data, size, offset); }

		// Getters
		virtual size_t GetSize() const { return m_StorageBuffer.GetSize(); }

		// Internal
		inline StorageBufferType& GetInternalStorageBuffer() { return m_StorageBuffer; }

	private:
		StorageBufferType m_StorageBuffer = {};
	};

}
