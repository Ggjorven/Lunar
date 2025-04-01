#pragma once

#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/Renderer/DescriptorSpec.hpp"
#include "Lunar/Internal/Renderer/PipelineSpec.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanDescriptor.hpp"

namespace Lunar::Internal
{

    class Pipeline;
    class CommandBuffer;

    ////////////////////////////////////////////////////////////////////////////////////
    // Selection
    ////////////////////////////////////////////////////////////////////////////////////
    template<Info::RenderingAPI API>
    struct DescriptorSetSelect;
    template<Info::RenderingAPI API>
    struct DescriptorSetsSelect;

    template<> struct DescriptorSetSelect<Info::RenderingAPI::Vulkan> { using Type = VulkanDescriptorSet; };
    template<> struct DescriptorSetsSelect<Info::RenderingAPI::Vulkan> { using Type = VulkanDescriptorSets; };

    using DescriptorSetType = typename DescriptorSetSelect<Info::g_RenderingAPI>::Type;
    using DescriptorSetsType = typename DescriptorSetsSelect<Info::g_RenderingAPI>::Type;

    ////////////////////////////////////////////////////////////////////////////////////
    // DescriptorSets
    ////////////////////////////////////////////////////////////////////////////////////
    class DescriptorSet
    {
    public:
        // Constructor & Destructor
        inline DescriptorSet(const RendererID renderer, uint8_t setID, const std::vector<VkDescriptorSet>& sets) { m_Descriptor.Init(renderer, setID, sets); }
        inline ~DescriptorSet() { m_Descriptor.Destroy(); }

        // Methods
        inline void Bind(Pipeline& pipeline, CommandBuffer& cmdBuf, PipelineBindPoint bindPoint = PipelineBindPoint::Graphics, const std::vector<uint32_t>& dynamicOffsets = { }) { m_Descriptor.Bind(pipeline, cmdBuf, bindPoint, dynamicOffsets); }

        inline void Upload(const std::vector<Uploadable>& elements) { m_Descriptor.Upload(elements); } // Uploads to the current frame descriptorset.

        // Internal
        inline DescriptorSetType& GetInternalDescriptorSet() { return m_Descriptor; }

    private:
        DescriptorSetType m_Descriptor = {};
    };

    class DescriptorSets
    {
    public:
        // Constructor & Destructor
        inline DescriptorSets(const RendererID renderer, const std::initializer_list<DescriptorSetRequest>& sets) { m_Descriptors.Init(renderer, sets); }
        inline ~DescriptorSets() { m_Descriptors.Destroy(); }

        // Setters & Getters
        inline void SetAmountOf(uint8_t setID, uint32_t amount) { m_Descriptors.SetAmountOf(setID, amount); }
        inline uint32_t GetAmountOf(uint8_t setID) const { m_Descriptors.GetAmountOf(setID); }
        
        inline const DescriptorSetLayout& GetLayout(uint8_t setID) const { m_Descriptors.GetLayout(setID); }
        inline std::vector<DescriptorSet*> GetSets(uint8_t setID) { m_Descriptors.GetSets(setID); }

        // Internal
        inline DescriptorSetsType& GetInternalDescriptorSets() { return m_Descriptors; }

    private:
        DescriptorSetsType m_Descriptors = {};
    };

}