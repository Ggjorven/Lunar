#pragma once

#include "Lunar/Internal/API/Vulkan/Vulkan.hpp"

#include "Lunar/Internal/Renderer/ShaderSpec.hpp"

namespace Lunar::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// Convert functions
	////////////////////////////////////////////////////////////////////////////////////
	ShaderStage VkShaderStageFlagsToShaderStage(VkShaderStageFlags stage);
	VkShaderStageFlags ShaderStageToVkShaderStageFlags(ShaderStage stage);

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanShaderCompiler
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanShaderCompiler
	{
	public:
		static std::vector<char> CompileGLSL(ShaderStage stage, const std::string& code);
	};

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanShader
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanShader
	{
	public:
		// Constructor & Destructor
		VulkanShader() = default;
		~VulkanShader() = default;

		// Init & Destroy
		void Init(const RendererID renderer, const ShaderSpecification& specs);
		void Destroy(const RendererID renderer);

		// Internal
		inline VkShaderModule GetShader(ShaderStage stage) { return m_Shaders[stage]; }
		inline const std::unordered_map<ShaderStage, VkShaderModule>& GetShaders() const { return m_Shaders; }

	private:
		// Static methods
		static VkShaderModule CreateShaderModule(const std::vector<char>& code);

	private:
		std::unordered_map<ShaderStage, VkShaderModule> m_Shaders = {};
	};

}
