#pragma once

#include "Lunar/Internal/API/Vulkan/Vulkan.hpp"

#include "Lunar/Internal/Renderer/DescriptorSpec.hpp"
#include "Lunar/Internal/Renderer/PipelineSpec.hpp"

#include <cstdint>
#include <vector>

namespace Lunar::Internal
{

	class Pipeline;
	class CommandBuffer;
	class DescriptorSet;

	class VulkanImage;
	class VulkanPipeline;
	class VulkanUniformBuffer;
	class VulkanStorageBuffer;

	////////////////////////////////////////////////////////////////////////////////////
	// Convert functions
	////////////////////////////////////////////////////////////////////////////////////
	DescriptorType VkDescriptorTypeToDescriptorType(VkDescriptorType type);
	VkDescriptorType DescriptorTypeToVkDescriptorType(DescriptorType type);
	DescriptorBindingFlags VkDescriptorBindingFlagsToDescriptorBindingFlags(VkDescriptorBindingFlags flags);
	VkDescriptorBindingFlags DescriptorBindingFlagsToVkDescriptorBindingFlags(DescriptorBindingFlags flags);

	////////////////////////////////////////////////////////////////////////////////////
	// Vulkan DescriptorSets
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanDescriptorSet
	{
	public:
		// Constructor & Destructor
		VulkanDescriptorSet() = default;
		~VulkanDescriptorSet() = default;

		// Init & Destroy
		void Init(const RendererID renderer, uint8_t setID, const std::vector<VkDescriptorSet>& sets);
		void Destroy(const RendererID renderer);

		// Methods
		void Bind(const RendererID renderer, Pipeline& pipeline, CommandBuffer& commandBuffer, PipelineBindPoint bindPoint, const std::vector<uint32_t>& dynamicOffsets);

		void Upload(const RendererID renderer, const std::vector<Uploadable>& elements);
		
		// Getters
		inline uint8_t GetSetID() const { return m_SetID; }

		// Internal
		inline VkDescriptorSet GetVkDescriptorSet(uint8_t index) const { return m_DescriptorSets[index]; }

	private:
		// Private methods
		void UploadImage(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorImageInfo>& imageInfos, VulkanImage& image, Descriptor descriptor, uint32_t arrayIndex, uint32_t frame);
		void UploadUniformBuffer(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorBufferInfo>& bufferInfos, VulkanUniformBuffer& buffer, Descriptor descriptor, uint32_t arrayIndex, uint32_t frame);
		void UploadStorageBuffer(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorBufferInfo>& bufferInfos, VulkanStorageBuffer& buffer, Descriptor descriptor, uint32_t arrayIndex, uint32_t frame);

	private:
		uint8_t m_SetID = 0;

		// Note: One for every frame in flight
		std::vector<VkDescriptorSet> m_DescriptorSets = { };
	};

	class VulkanDescriptorSets
	{
	public:
		// Constructor & Destructor
		VulkanDescriptorSets() = default;
		~VulkanDescriptorSets() = default;

		// Init & Destroy
		void Init(const RendererID renderer, const std::initializer_list<DescriptorSetRequest>& specs);
		void Destroy(const RendererID renderer);

		// Setters & Getters
		void SetAmountOf(const RendererID renderer, uint8_t setID, uint32_t amount);
		uint32_t GetAmountOf(uint8_t setID) const;

		const DescriptorSetLayout& GetLayout(uint8_t setID) const;
		std::vector<DescriptorSet*> GetSets(uint8_t setID);

	private:
		// Private methods
		void CreateDescriptorSetLayout(const RendererID renderer, uint8_t setID);
		void CreateDescriptorPool(const RendererID renderer, uint8_t setID, uint32_t amount);
		void CreateDescriptorSets(const RendererID renderer, uint8_t setID, uint32_t amount);
		void ConvertToVulkanDescriptorSets(const RendererID renderer, uint8_t setID, uint32_t amount, std::vector<VkDescriptorSet>& sets);

	private:
		std::vector<DescriptorSetLayout> m_OriginalLayouts = { };
		std::vector<std::vector<VulkanDescriptorSet>> m_DescriptorSets = { };

		std::vector<VkDescriptorSetLayout> m_DescriptorLayouts = { };
		std::vector<VkDescriptorPool> m_DescriptorPools = { };

		friend class VulkanPipeline;
	};

}
