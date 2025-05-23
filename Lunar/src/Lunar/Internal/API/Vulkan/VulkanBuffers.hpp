#pragma once

#include "Lunar/Internal/API/Vulkan/Vulkan.hpp"

#include "Lunar/Internal/Renderer/RendererSpec.hpp"
#include "Lunar/Internal/Renderer/BuffersSpec.hpp"

namespace Lunar::Internal
{

	class VertexBuffer;
	class CommandBuffer;

    class VulkanSwapChain;
    class VulkanDescriptorSet;

    ////////////////////////////////////////////////////////////////////////////////////
    // Convert functions
    ////////////////////////////////////////////////////////////////////////////////////
    VertexInputRate VkVertexInputRateToVertexInputRate(VkVertexInputRate input);
    VkVertexInputRate VertexInputRateToVkVertexInputRate(VertexInputRate input);
    DataType VkFormatToDataType(VkFormat format);
    VkFormat DataTypeToVkFormat(DataType dataType);

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanVertexBuffer
    ////////////////////////////////////////////////////////////////////////////////////
	class VulkanVertexBuffer
	{
	public:
		// Constructor & Destructor
		VulkanVertexBuffer() = default;
		~VulkanVertexBuffer() = default;

		// Init & Destroy
		void Init(const RendererID renderer, const BufferSpecification& specs, void* data, size_t size);
		void Destroy(const RendererID renderer);

		// Methods
		void Bind(const RendererID renderer, CommandBuffer& cmdBuf) const;

		void SetData(const RendererID renderer, void* data, size_t size, size_t offset);

		// Static methods
		static void Bind(const RendererID renderer, CommandBuffer& cmdBuf, const std::vector<VertexBuffer*>& buffers);

	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;

		size_t m_BufferSize = 0;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanIndexBuffer
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanIndexBuffer
	{
	public:
		enum class Type : uint8_t { None = 0, UInt8, UInt16, UInt32 };
	public:
		// Constructor & Destructor
		VulkanIndexBuffer() = default;
		~VulkanIndexBuffer() = default;

		// Init & Destroy
		void Init(const RendererID renderer, const BufferSpecification& specs, uint8_t* indices, uint32_t count);
		void Init(const RendererID renderer, const BufferSpecification& specs, uint16_t* indices, uint32_t count);
		void Init(const RendererID renderer, const BufferSpecification& specs, uint32_t* indices, uint32_t count);
		void Destroy(const RendererID renderer);

		// Methods
		void Bind(const RendererID renderer, CommandBuffer& cmdBuf) const;

		void SetData(const RendererID renderer, uint8_t* indices, uint32_t count, size_t countOffset);
		void SetData(const RendererID renderer, uint16_t* indices, uint32_t count, size_t countOffset);
		void SetData(const RendererID renderer, uint32_t* indices, uint32_t count, size_t countOffset);

		// Getters
		inline uint32_t GetCount() const { return m_Count; }

	private:
		// Private methods
		void SetData(const RendererID renderer, void* indices, size_t size, size_t offset);

	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;

		Type m_Type = Type::None;
		uint32_t m_Count = 0;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanUniformBuffer
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanUniformBuffer
	{
	public:
		// Constructor & Destructor
		VulkanUniformBuffer() = default;
		~VulkanUniformBuffer() = default;

		// Init & Destroy
		void Init(const RendererID renderer, const BufferSpecification& specs, size_t dataSize);
		void Destroy(const RendererID renderer);

		// Methods
		void SetData(const RendererID renderer, void* data, size_t size, size_t offset);

		// Getters
		inline size_t GetSize() const { return m_Size; }

	private:
		std::vector<VkBuffer> m_Buffers = { };
		std::vector<VmaAllocation> m_Allocations = { };

		size_t m_Size = 0;

		friend class VulkanDescriptorSet;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanDynamicUniformBuffer
	////////////////////////////////////////////////////////////////////////////////////
	/* // TODO: Implement
	class VulkanDynamicUniformBuffer
	{
	public:
		VulkanDynamicUniformBuffer(uint32_t elements, size_t sizeOfOneElement);
		virtual ~VulkanDynamicUniformBuffer();

		void SetData(void* data, size_t size) override;

		void SetDataIndexed(uint32_t index, void* data, size_t size) override;
		void UploadIndexedData() override;

		inline uint32_t GetAmountOfElements() const override { return m_ElementCount; }
		inline size_t GetAlignment() const override { return m_AlignmentOfOneElement; }

		void Upload(Ref<DescriptorSet> set, Descriptor element) override;
		void Upload(Ref<DescriptorSet> set, Descriptor element, size_t offset) override;

	private:
		std::vector<VkBuffer> m_Buffers = { };
		std::vector<VmaAllocation> m_Allocations = { };

		uint32_t m_ElementCount = 0;
		size_t m_SizeOfOneElement = 0;
		size_t m_AlignmentOfOneElement = 0;

		std::vector<std::pair<void*, size_t>> m_IndexedData = { };

		friend class VulkanDescriptorSet;
	};
	*/

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanStorageBuffer
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanStorageBuffer
	{
	public:
		// Constructor & Destructor
		VulkanStorageBuffer() = default;
		~VulkanStorageBuffer() = default;

		// Init & Destroy
		void Init(const RendererID renderer, const BufferSpecification& specs, size_t dataSize);
		void Destroy(const RendererID renderer);

		// Methods
		void SetData(const RendererID renderer, void* data, size_t size, size_t offset);

		// Getters
		inline size_t GetSize() const { return m_Size; }

	private:
		std::vector<VkBuffer> m_Buffers = { };
		std::vector<VmaAllocation> m_Allocations = { };

		size_t m_Size = 0;

		friend class VulkanDescriptorSet;
	};

}