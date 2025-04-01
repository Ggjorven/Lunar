#pragma once

#include "Lunar/Internal/Enum/Bitwise.hpp"

#include "Lunar/Internal/Renderer/ShaderSpec.hpp"

#include <cstdint>
#include <string>
#include <variant>
#include <unordered_map>
#include <unordered_set>

namespace Lunar::Internal
{

    class Image;
    class UniformBuffer;
    class StorageBuffer;

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

    // Note: You can think of a descriptor as a uniform or some variable in the shader
    struct Descriptor
    {
    public:
        static constexpr const uint32_t MaxBindlessResources = 16536u;
    public:
        std::string Name = "Empty";
        uint32_t Binding = 0;
        DescriptorType Type = DescriptorType::None;
        ShaderStage Stage = ShaderStage::None;
        uint32_t Count = 1;
        DescriptorBindingFlags BindingFlags = DescriptorBindingFlags::None;

        Descriptor() = default;
        Descriptor(DescriptorType type, uint32_t binding, const std::string& name, ShaderStage stage, uint32_t count = 1, DescriptorBindingFlags bindingFlags = DescriptorBindingFlags::None);
        ~Descriptor() = default;
    };

    struct DescriptorSetLayout
    {
    public:
        uint8_t SetID = 0;
        std::unordered_map<std::string, Descriptor> Descriptors = { };

    public:
        DescriptorSetLayout() = default;
        DescriptorSetLayout(uint8_t setID, const std::vector<Descriptor>& descriptors);
        DescriptorSetLayout(uint8_t setID, const std::initializer_list<Descriptor>& descriptors);
        ~DescriptorSetLayout() = default;

        bool ContainsBindless() const;

        Descriptor GetDescriptorByName(const std::string& name) const;
        std::unordered_set<DescriptorType> UniqueTypes() const;
        uint32_t AmountOf(DescriptorType type) const;
    };

    struct DescriptorSetRequest
    {
    public:
        uint32_t Amount = 1;
        DescriptorSetLayout Layout = {};
    };

    struct Uploadable
    {
    public:
        using Type = std::variant<Image*, UniformBuffer*, StorageBuffer*>;
    public:
        Type Value;
        Descriptor Element;
        uint32_t ArrayIndex = 0;
    };

}
