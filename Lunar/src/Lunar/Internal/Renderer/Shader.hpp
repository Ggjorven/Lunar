#pragma once

#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/Renderer/RendererSpec.hpp"
#include "Lunar/Internal/Renderer/ShaderSpec.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanShader.hpp"

#include <cstdint>
#include <vector>
#include <string>
#include <filesystem>

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Selection
    ////////////////////////////////////////////////////////////////////////////////////
    template<Info::RenderingAPI API>
    struct ShaderCompilerSelect;
    template<Info::RenderingAPI API>
    struct ShaderSelect;

    template<> struct ShaderCompilerSelect<Info::RenderingAPI::Vulkan> { using Type = VulkanShaderCompiler; };
    template<> struct ShaderSelect<Info::RenderingAPI::Vulkan> { using Type = VulkanShader; };

    using ShaderCompilerType = typename ShaderCompilerSelect<Info::g_RenderingAPI>::Type;
    using ShaderType = typename ShaderSelect<Info::g_RenderingAPI>::Type;

    ////////////////////////////////////////////////////////////////////////////////////
    // Compiler
    ////////////////////////////////////////////////////////////////////////////////////
    class ShaderCompiler
    {
    public:
        inline static std::vector<char> CompileGLSL(ShaderStage stage, const std::string& code) { return ShaderCompilerType::CompileGLSL(stage, code); }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Shader
    ////////////////////////////////////////////////////////////////////////////////////
    class Shader
    {
    public:
		// Constructors & Destructor
		inline Shader() = default;
		inline Shader(const RendererID renderer, const ShaderSpecification& specs) { Init(renderer, specs); }
        inline ~Shader() = default;

        // Init & Destroy
		inline void Init(const RendererID renderer, const ShaderSpecification& specs) { m_Shader.Init(renderer, specs); }
        inline void Destroy(const RendererID renderer) { m_Shader.Destroy(renderer); }

		// Static methods
        static std::string ReadGLSL(const std::filesystem::path& path);
        static std::vector<char> ReadSPIRV(const std::filesystem::path& path);

        // Internal
        inline ShaderType& GetInternalShader() { return m_Shader; }

    private:
		ShaderType m_Shader = {};
    };

}