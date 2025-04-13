#include "lupch.h"
#include "Descriptor.hpp"
#include "DescriptorSpec.hpp"

#include "Lunar/Internal/IO/Print.hpp"

namespace Lunar::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// Descriptor Spec
	////////////////////////////////////////////////////////////////////////////////////
    Descriptor::Descriptor(DescriptorType type, uint32_t binding, const std::string& name, ShaderStage stage, uint32_t count, DescriptorBindingFlags bindingFlags)
        : Name(name), Binding(binding), Type(type), Stage(stage), Count(count), BindingFlags(bindingFlags)
    {
    }

    DescriptorSetLayout::DescriptorSetLayout(uint8_t setID, const std::vector<Descriptor>& descriptors)
        : SetID(setID)
    {
        for (auto& descriptor : descriptors)
            Descriptors[descriptor.Name] = descriptor;
    }

    DescriptorSetLayout::DescriptorSetLayout(uint8_t setID, const std::initializer_list<Descriptor>& descriptors)
        : DescriptorSetLayout(setID, std::vector(descriptors))
    {
    }

    bool DescriptorSetLayout::ContainsBindless() const
    {
        for (const auto& [name, descriptor] : Descriptors)
        {
            if (descriptor.BindingFlags != DescriptorBindingFlags::None)
                return true;
        }

        return false;
    }

    Descriptor DescriptorSetLayout::GetDescriptorByName(const std::string& name) const
    {
        auto it = Descriptors.find(name);
        if (it == Descriptors.end()) [[unlikely]]
        {
            LU_LOG_ERROR("Failed to find descriptor by name: '{0}'", name);
            return {};
        }

        return it->second;
    }

    std::unordered_set<DescriptorType> DescriptorSetLayout::UniqueTypes() const
    {
        std::unordered_set<DescriptorType> unique = { };

        for (const auto& [name, descriptor] : Descriptors)
            unique.insert(descriptor.Type);

        return unique;
    }

    uint32_t DescriptorSetLayout::AmountOf(DescriptorType type) const
    {
        uint32_t count = 0;

        for (const auto& [name, descriptor] : Descriptors)
        {
            if (descriptor.Type == type)
                count++;
        }

        return count;
    }

    uint32_t DescriptorSetLayout::CountOf(DescriptorType type) const
    {
        uint32_t count = 0;

        for (const auto& [name, descriptor] : Descriptors)
        {
            if (descriptor.Type == type)
				count += descriptor.Count;
        }

        return count;
    }

}
