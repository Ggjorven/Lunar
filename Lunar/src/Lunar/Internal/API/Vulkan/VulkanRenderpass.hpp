#pragma once

#include "Lunar/Internal/API/Vulkan/Vulkan.hpp"

#include "Lunar/Internal/Renderer/RenderpassSpec.hpp"
#include "Lunar/Internal/Renderer/CommandBuffer.hpp"

namespace Lunar::Internal
{

	class VulkanRenderer;

	////////////////////////////////////////////////////////////////////////////////////
	// Convert functions
	////////////////////////////////////////////////////////////////////////////////////
	LoadOperation VkAttachmentLoadOpToLoadOperation(VkAttachmentLoadOp loadOp);
	VkAttachmentLoadOp LoadOperationToVkAttachmentLoadOp(LoadOperation loadOp);
	StoreOperation VkAttachmentStoreOpToStoreOperation(VkAttachmentStoreOp storeOp);
	VkAttachmentStoreOp StoreOperationToVkAttachmentStoreOp(StoreOperation storeOp);

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanRenderpass
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanRenderpass
	{
	public:
		// Constructor & Destructor
		VulkanRenderpass() = default;
		~VulkanRenderpass() = default;

		// Init & Destroy
		void Init(const RendererID renderer, const RenderpassSpecification& specs, CommandBuffer* cmdBuf);
		void Destroy();

		// The Begin, End & Submit methods are in the Renderer

		// Methods
		void Resize(uint32_t width, uint32_t height);

		// Getters
		Vec2<uint32_t> GetSize() const;

		inline const RenderpassSpecification& GetSpecification() const { return m_Specification; }
		inline CommandBuffer& GetCommandBuffer() { return *m_CommandBuffer; }

		// Internal
		inline VkRenderPass GetVkRenderPass() const { return m_RenderPass; }
		inline std::vector<VkFramebuffer>& GetVkFrameBuffers() { return m_Framebuffers; };

	private:
		// Private methods
		void CreateRenderpass();
		void CreateFramebuffers(uint32_t width, uint32_t height);
		void DestroyRenderpass();

		std::vector<VkSubpassDependency> GetDependencies(RenderpassUsage usage);

	private:
		RendererID m_RendererID = 0;
		RenderpassSpecification m_Specification = {};

		CommandBuffer* m_CommandBuffer = nullptr;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> m_Framebuffers = { };

		friend class VulkanRenderer;
	};

}
