#pragma once

#include "Lunar/Internal/API/Vulkan/Vulkan.hpp"

#include "Lunar/Internal/Renderer/RendererSpec.hpp"
#include "Lunar/Internal/Renderer/PipelineSpec.hpp"

namespace Lunar::Internal
{

	class DescriptorSets;
	class CommandBuffer;
	class Shader;
	class Renderpass;

	class VulkanDescriptorSets;

	////////////////////////////////////////////////////////////////////////////////////
	// Convert functions
	////////////////////////////////////////////////////////////////////////////////////
	PipelineStage VkPipelineStageToPipelineStage(VkPipelineStageFlags stage);
	VkPipelineStageFlags PipelineStageToVkPipelineStage(PipelineStage stage);
	CullingMode VkCullModeFlagsToCullingMode(VkCullModeFlags mode);
	VkCullModeFlags CullingModeToVkCullModeFlags(CullingMode mode);
	PolygonMode VkPolygonModeToPolygonMode(VkPolygonMode mode);
	VkPolygonMode PolygonModeToVkPolygonMode(PolygonMode mode);
	PipelineBindPoint VkPipelineBindPointToPipelineBindPoint(VkPipelineBindPoint bindPoint);
	VkPipelineBindPoint PipelineBindPointToVkPipelineBindPoint(PipelineBindPoint bindPoint);

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanPipeline
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanPipeline
	{
	public:
		// Constructor & Destructor
		VulkanPipeline() = default;
		~VulkanPipeline() = default;

		// Init & Destroy
		void Init(const RendererID renderer, const PipelineSpecification& specs, DescriptorSets& sets, Shader& shader);
		void Init(const RendererID renderer, const PipelineSpecification& specs, DescriptorSets& sets, Shader& shader, Renderpass& renderpass);
		void Destroy();

		// Methods
		void Use(CommandBuffer& cmdBuf, PipelineBindPoint bindPoint);

		void PushConstant(CommandBuffer& cmdBuf, ShaderStage stage, void* data);
		void PushConstant(CommandBuffer& cmdBuf, ShaderStage stage, void* data, size_t offset, size_t size);

		void DispatchCompute(CommandBuffer& cmdBuf, uint32_t width, uint32_t height, uint32_t depth);

		// Getters
		inline const PipelineSpecification& GetSpecification() const { return m_Specification; };

		// Internal
		inline const VkPipeline GetVkPipeline() const { return m_Pipeline; }
		inline const VkPipelineLayout GetVkPipelineLayout() const { return m_PipelineLayout; }

	private:
		// Static methods
		void CreateGraphicsPipeline(DescriptorSets& sets, Shader& shader, Renderpass* renderpass);
		void CreateComputePipeline(DescriptorSets& sets, Shader& shader);
		void CreateRayTracingPipelineKHR(DescriptorSets& sets, Shader& shader);
		void CreateRayTracingPipelineNV(DescriptorSets& sets, Shader& shader);

		VkVertexInputBindingDescription GetBindingDescription(); // TODO: Make Instanced support
		std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

	private:
		RendererID m_RendererID = 0;
		PipelineSpecification m_Specification = {};

		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

		friend class VulkanDescriptorSets;
	};

}