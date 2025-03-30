#pragma once

#include "Lunar/Internal/Enum/Bitwise.hpp"

#include <cstdint>

namespace Hz
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Descriptor spec
    ////////////////////////////////////////////////////////////////////////////////////
    enum class DescriptorType : uint8_t
    {
        None = 0,
        Sampler,
        CombinedImageSampler,
        SampledImage,
        StorageImage,
        UniformTexelBuffer,
        StorageTexelBuffer,
        UniformBuffer,
        StorageBuffer,
        DynamicUniformBuffer,
        DynamicStorageBuffer,
        InputAttachment,
        UniformBlock,
        AccelerationStructureKHR,
        AccelerationStructureNV,
        SampleWeightImageQCOM,
        BlockMatchImageQCOM,
        MutableEXT,

        UniformBlockEXT = UniformBlock,
        MutableValve = MutableEXT,
    };

    enum class DescriptorBindingFlags : uint8_t
    {
        None = 0,
        UpdateAfterBind = 1 << 0,
        UpdateUnusedWhilePending = 1 << 1,
        PartiallyBound = 1 << 2,
        VariableDescriptorCount = 1 << 3,

        UpdateAfterBindEXT = UpdateAfterBind,
        UpdateUnusedWhilePendingEXT = UpdateUnusedWhilePending,
        PartiallyBoundEXT = PartiallyBound,
        VariableDescriptorCountEXT = VariableDescriptorCount,

        // Note: Most often used for sampler2D[]
        Default = UpdateAfterBind | PartiallyBound
    };
    LU_ENABLE_BITWISE(DescriptorBindingFlags);

}
