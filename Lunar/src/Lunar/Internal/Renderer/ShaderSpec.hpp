#pragma once

#include "Lunar/Internal/Enum/Bitwise.hpp"

#include <cstdint>
#include <unordered_map>

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
	// Shader specs
    ////////////////////////////////////////////////////////////////////////////////////
    enum class ShaderStage : uint16_t
    {
        None = 0,
        Vertex = 1 << 0,
        TessellationControl = 1 << 1,
        TessellationEvaluation = 1 << 2,
        Geometry = 1 << 3,
        Fragment = 1 << 4,
        Compute = 1 << 5,
        AllGraphics = Vertex | TessellationControl | TessellationEvaluation | Geometry | Fragment,

        RayGenKHR = 1 << 6,
        AnyHitKHR = 1 << 7,
        ClosestHitKHR = 1 << 8,
        MissKHR = 1 << 9,
        IntersectionKHR = 1 << 10,
        CallableKHR = 1 << 11,
        TaskEXT = 1 << 12,
        MeshEXT = 1 << 13,
        SubpassShadingHuawei = 1 << 14,
        ClusterCullingHuawei = 1 << 15,

        RayGenNV = RayGenKHR,
        AnyHitNV = AnyHitKHR,
        ClosestHitNV = ClosestHitKHR,
        MissNV = MissKHR,
        IntersectionNV = IntersectionKHR,
        CallableNV = CallableKHR,
        TaskNV = TaskEXT,
        MeshNV = MeshEXT
    };
	LU_ENABLE_BITWISE(ShaderStage);

    struct ShaderSpecification
    {
    public:
        // Note: Currently the shader code is required to be in the SPIR-V format.
        std::unordered_map<ShaderStage, std::vector<char>> Shaders = { };
    };

}