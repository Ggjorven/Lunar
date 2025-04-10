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
		inline DescriptorSet() = default;
        inline DescriptorSet(const RendererID renderer, uint8_t setID, const std::vector<VkDescriptorSet>& sets) { Init(renderer, setID, sets); }
        inline ~DescriptorSet() = default;

		// Init & Destroy
        inline void Init(const RendererID renderer, uint8_t setID, const std::vector<VkDescriptorSet>& sets) { m_Descriptor.Init(renderer, setID, sets); }
        inline void Destroy(const RendererID renderer) { m_Descriptor.Destroy(renderer); }

        // Methods
        inline void Bind(const RendererID renderer, Pipeline& pipeline, CommandBuffer& cmdBuf, PipelineBindPoint bindPoint = PipelineBindPoint::Graphics, const std::vector<uint32_t>& dynamicOffsets = { }) { m_Descriptor.Bind(renderer, pipeline, cmdBuf, bindPoint, dynamicOffsets); }

        inline void Upload(const RendererID renderer, const std::vector<Uploadable>& elements) { m_Descriptor.Upload(renderer, elements); } // Uploads to the current frame descriptorset.

        // Internal
        inline DescriptorSetType& GetInternalDescriptorSet() { return m_Descriptor; }

    private:
        DescriptorSetType m_Descriptor = {};
    };

    class DescriptorSets
    {
    public:
        // Constructors & Destructor
		inline DescriptorSets() = default;
        inline DescriptorSets(const RendererID renderer, const std::initializer_list<DescriptorSetRequest>& sets) { Init(renderer, sets); }
        inline ~DescriptorSets() = default;

        // Init & Destroy
        inline void Init(const RendererID renderer, const std::initializer_list<DescriptorSetRequest>& sets) { m_Descriptors.Init(renderer, sets); }
		inline void Destroy(const RendererID renderer) { m_Descriptors.Destroy(renderer); }

        // Setters & Getters
        inline void SetAmountOf(const RendererID renderer, uint8_t setID, uint32_t amount) { m_Descriptors.SetAmountOf(renderer, setID, amount); }
        inline uint32_t GetAmountOf(uint8_t setID) const { return m_Descriptors.GetAmountOf(setID); }
        
        inline const DescriptorSetLayout& GetLayout(uint8_t setID) const { return m_Descriptors.GetLayout(setID); }
        inline std::vector<DescriptorSet*> GetSets(uint8_t setID) { return m_Descriptors.GetSets(setID); }

        // Internal
        inline DescriptorSetsType& GetInternalDescriptorSets() { return m_Descriptors; }

    private:
        DescriptorSetsType m_Descriptors = {};
    };

}