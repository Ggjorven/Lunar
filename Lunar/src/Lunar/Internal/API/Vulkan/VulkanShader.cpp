#include "lupch.h"
#include "VulkanShader.hpp"

#include "Lunar/Internal/IO/Print.hpp"

#include "Lunar/Internal/Renderer/Renderer.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanRenderer.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"

#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
	// VulkanShaderCompiler
    ////////////////////////////////////////////////////////////////////////////////////
    static uint32_t ShaderVersion()
    {
        switch (std::get<1>(g_VkVersion))
        {
        case 0:             return shaderc_env_version_vulkan_1_0;
        case 1:             return shaderc_env_version_vulkan_1_1;
        case 2:             return shaderc_env_version_vulkan_1_2;
        case 3:             return shaderc_env_version_vulkan_1_3;
        case 4:             return shaderc_env_version_vulkan_1_4;

        default:            return shaderc_env_version_vulkan_1_3;
        }
    }

    static shaderc_shader_kind ShaderStageToShaderCType(ShaderStage stage)
    {
        switch (stage)
        {
        case ShaderStage::Vertex:                       return shaderc_glsl_vertex_shader;
        case ShaderStage::Fragment:                     return shaderc_glsl_fragment_shader;
        case ShaderStage::Compute:                      return shaderc_glsl_compute_shader;
        case ShaderStage::Geometry:                     return shaderc_glsl_geometry_shader;
        case ShaderStage::TessellationControl:          return shaderc_glsl_tess_control_shader;
        case ShaderStage::TessellationEvaluation:       return shaderc_glsl_tess_evaluation_shader;

        case ShaderStage::RayGenKHR: /*Also NV*/        return shaderc_glsl_raygen_shader;
        case ShaderStage::AnyHitKHR: /*Also NV*/        return shaderc_glsl_anyhit_shader;
        case ShaderStage::ClosestHitKHR: /*Also NV*/    return shaderc_glsl_closesthit_shader;
        case ShaderStage::MissKHR: /*Also NV*/          return shaderc_glsl_miss_shader;
        case ShaderStage::IntersectionKHR: /*Also NV*/  return shaderc_glsl_intersection_shader;
        case ShaderStage::CallableKHR: /*Also NV*/      return shaderc_glsl_callable_shader;

        case ShaderStage::TaskEXT: /*Also NV*/          return shaderc_glsl_task_shader;
        case ShaderStage::MeshEXT: /*Also NV*/          return shaderc_glsl_mesh_shader;

        default:
            LU_LOG_ERROR("[VulkanShaderCompiler] ShaderStage passed in is currently not supported.");
            break;
        }

        // Return vertex shader by default.
        return shaderc_glsl_vertex_shader;
    }

    std::vector<char> VulkanShaderCompiler::CompileGLSL(ShaderStage stage, const std::string& code)
    {
        LU_VERIFY((!code.empty()), "[VulkanShaderCompiler] Empty string passed in as shader code.");

        shaderc::Compiler compiler = {};
        shaderc::CompileOptions options = {};
        options.SetTargetEnvironment(shaderc_target_env_vulkan, ShaderVersion());

        shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(code, ShaderStageToShaderCType(stage), "", options);

        LU_ASSERT((module.GetCompilationStatus() == shaderc_compilation_status_success), std::format("[VulkanShaderCompiler] Error compiling shader: {0}", module.GetErrorMessage()));

        // Convert SPIR-V code to vector<char>
        const uint32_t* data = module.cbegin();
        const size_t numWords = module.cend() - module.cbegin();
        const size_t sizeInBytes = numWords * sizeof(uint32_t);
        const char* bytes = reinterpret_cast<const char*>(data);

        return std::vector<char>(bytes, bytes + sizeInBytes);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Init & Destroy
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanShader::Init(const RendererID renderer, const ShaderSpecification& specs)
    {
        m_RendererID = renderer;

        for (const auto& [stage, code] : specs.Shaders)
            m_Shaders[stage] = CreateShaderModule(code);
    }

    void VulkanShader::Destroy()
    {
        // Note: This might be redundant since shaders are only usefuls when a pipeline needs to be created.
        // But better safe than sorry.
        Renderer::GetRenderer(m_RendererID).Free([shaders = m_Shaders]()
        {
            auto device = VulkanContext::GetVulkanDevice().GetVkDevice();

            for (auto& [stage, shader] : shaders)
                vkDestroyShaderModule(device, shader, nullptr);
        });
    }

    ////////////////////////////////////////////////////////////////////////////////////
	// Static methods
    ////////////////////////////////////////////////////////////////////////////////////
    VkShaderModule VulkanShader::CreateShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateShaderModule(VulkanContext::GetVulkanDevice().GetVkDevice(), &createInfo, nullptr, &shaderModule));

        return shaderModule;
    }

}
