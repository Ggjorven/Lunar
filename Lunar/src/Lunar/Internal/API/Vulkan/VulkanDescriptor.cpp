#include "lupch.h"
#include "VulkanDescriptor.hpp"

#include "Lunar/Internal/IO/Print.hpp"
#include "Lunar/Internal/Utils/Profiler.hpp"

#include "Lunar/Internal/Renderer/Renderer.hpp"
#include "Lunar/Internal/Renderer/Image.hpp"
#include "Lunar/Internal/Renderer/Pipeline.hpp"
#include "Lunar/Internal/Renderer/CommandBuffer.hpp"

#include "Lunar/Internal/API/Vulkan/VulkanContext.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanImage.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanShader.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanPipeline.hpp"
#include "Lunar/Internal/API/Vulkan/VulkanCommandBuffer.hpp"

#include <format>

namespace Lunar::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Init & Destroy
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanDescriptorSet::Init(const RendererID renderer, uint8_t setID, const std::vector<VkDescriptorSet>& sets)
    {
        m_RendererID = renderer;
        m_SetID = setID;

        m_DescriptorSets = sets;
    }

    void VulkanDescriptorSet::Destroy()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanDescriptorSet::Bind(Pipeline& pipeline, CommandBuffer& commandBuffer, PipelineBindPoint bindPoint, const std::vector<uint32_t>& dynamicOffsets)
    {
        // TODO: Uncomment all

        LU_PROFILE("VkDescriptorSet::Bind");
        uint32_t currentFrame = VulkanRenderer::GetRenderer(m_RendererID).GetVulkanSwapChain().GetCurrentFrame();
        //auto vkPipelineLayout = pipeline.GetInternalPipeline().GetVkPipelineLayout();
        auto vkCmdBuf = commandBuffer.GetInternalCommandBuffer().GetVkCommandBuffer(currentFrame);

        //vkCmdBindDescriptorSets(vkCmdBuf, PipelineBindPointToVkPipelineBindPoint(bindPoint), vkPipelineLayout, m_SetID, 1, &m_DescriptorSets[currentFrame], static_cast<uint32_t>(dynamicOffsets.size()), dynamicOffsets.data());
    }

    void VulkanDescriptorSet::Upload(const std::vector<Uploadable>& elements)
    {
        LU_PROFILE("VkDescriptorSet::Upload");
        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(elements.size());

        std::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.reserve(elements.size());
        std::vector<VkDescriptorBufferInfo> bufferInfos;
        bufferInfos.reserve(elements.size());

        uint32_t currentFrame = VulkanRenderer::GetRenderer(m_RendererID).GetVulkanSwapChain().GetCurrentFrame();

        for (const auto& [uploadable, descriptor, arrayIndex] : elements)
        {
            std::visit([&, descriptor, arrayIndex](auto&& arg)
                {
                    using T = std::decay_t<decltype(arg)>;

                    // TODO: Uncomment
                    if constexpr (std::is_same_v<T, Image*>)                  UploadImage(writes, imageInfos, arg->GetInternalImage(), descriptor, arrayIndex, currentFrame);
                    //else if constexpr (std::is_same_v<T, UniformBuffer*>)     UploadUniformBuffer(writes, bufferInfos, arg->GetInternalUniformBuffer(), descriptor, arrayIndex, //currentFrame);
                    //else if constexpr (std::is_same_v<T, StorageBuffer*>)     UploadStorageBuffer(writes, bufferInfos, arg->GetInternalStorageBuffer(), descriptor, arrayIndex, currentFrame);
                }, uploadable);
        }

        {
            LU_PROFILE("VkDescriptorSet::Upload::UpdateCmd");
            vkUpdateDescriptorSets(VulkanContext::GetVulkanDevice().GetVkDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanDescriptorSet::UploadImage(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorImageInfo>& imageInfos, VulkanImage& image, Descriptor descriptor, uint32_t arrayIndex, uint32_t frame)
    {
        VkDescriptorImageInfo& imageInfo = imageInfos.emplace_back();
        imageInfo.imageLayout = ImageLayoutToVkImageLayout(image.m_ImageSpecification.Layout);
        imageInfo.imageView = image.m_ImageView;
        imageInfo.sampler = image.m_Sampler;

        VkWriteDescriptorSet& descriptorWrite = writes.emplace_back();
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSets[frame];
        descriptorWrite.dstBinding = descriptor.Binding;
        descriptorWrite.dstArrayElement = arrayIndex; // Is 0 when not set.
        descriptorWrite.descriptorType = DescriptorTypeToVkDescriptorType(descriptor.Type);
        descriptorWrite.descriptorCount = 1; // descriptor.Count;
        descriptorWrite.pImageInfo = &imageInfo;
    }

    void VulkanDescriptorSet::UploadUniformBuffer(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorBufferInfo>& bufferInfos, VulkanUniformBuffer& buffer, Descriptor descriptor, uint32_t arrayIndex, uint32_t frame)
    {
        /* // TODO: Uncomment
        VkDescriptorBufferInfo& bufferInfo = bufferInfos.emplace_back();
        bufferInfo.buffer = buffer.m_Buffers[frame];
        bufferInfo.offset = 0;
        bufferInfo.range = buffer.m_Size;

        VkWriteDescriptorSet& descriptorWrite = writes.emplace_back();
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSets[frame];
        descriptorWrite.dstBinding = descriptor.Binding;
        descriptorWrite.dstArrayElement = arrayIndex; // Is 0 when not set.
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1; // descriptor.Count;
        descriptorWrite.pBufferInfo = &bufferInfo;
        */
    }

    void VulkanDescriptorSet::UploadStorageBuffer(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorBufferInfo>& bufferInfos, VulkanStorageBuffer& buffer, Descriptor descriptor, uint32_t arrayIndex, uint32_t frame)
    {
        /* // TODO: Uncomment
        VkDescriptorBufferInfo& bufferInfo = bufferInfos.emplace_back();
        bufferInfo.buffer = buffer.m_Buffers[frame];
        bufferInfo.offset = 0;
        bufferInfo.range = buffer.m_Size;

        VkWriteDescriptorSet& descriptorWrite = writes.emplace_back();
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSets[frame];
        descriptorWrite.dstBinding = descriptor.Binding;
        descriptorWrite.dstArrayElement = arrayIndex; // Is 0 when not set.
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrite.descriptorCount = 1; // descriptor.Count;
        descriptorWrite.pBufferInfo = &bufferInfo;
        */
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Init & Destroy
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanDescriptorSets::Init(const RendererID renderer, const std::initializer_list<DescriptorSetRequest>& sets)
    {
        m_RendererID = renderer;

        size_t maxSetID = 0;
        for (auto& group : sets)
            maxSetID = (group.Layout.SetID > maxSetID ? group.Layout.SetID : maxSetID);

        m_OriginalLayouts.resize(maxSetID + 1);
        m_DescriptorSets.resize(maxSetID + 1);
        m_DescriptorLayouts.resize(maxSetID + 1);
        m_DescriptorPools.resize(maxSetID + 1);

        for (auto& group : sets)
        {
            m_OriginalLayouts[group.Layout.SetID] = group.Layout;

            CreateDescriptorSetLayout(group.Layout.SetID);
            CreateDescriptorPool(group.Layout.SetID, group.Amount);
            CreateDescriptorSets(group.Layout.SetID, group.Amount);
        }
    }

    void VulkanDescriptorSets::Destroy()
    {
        Renderer::GetRenderer(m_RendererID).Free([descriptorPools = m_DescriptorPools, descriptorLayouts = m_DescriptorLayouts]()
        {
            auto device = VulkanContext::GetVulkanDevice().GetVkDevice();

            for (auto& pool : descriptorPools)
                vkDestroyDescriptorPool(device, pool, nullptr);

            for (auto& layout : descriptorLayouts)
                vkDestroyDescriptorSetLayout(device, layout, nullptr);
        });
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Setters & Getters
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanDescriptorSets::SetAmountOf(uint8_t setID, uint32_t amount)
    {
        vkDestroyDescriptorPool(VulkanContext::GetVulkanDevice().GetVkDevice(), m_DescriptorPools[setID], nullptr);

        CreateDescriptorPool(setID, amount);
        CreateDescriptorSets(setID, amount);
    }

    uint32_t VulkanDescriptorSets::GetAmountOf(uint8_t setID) const
    {
        LU_VERIFY(!m_OriginalLayouts[setID].Descriptors.empty(), std::format("[VkDescriptorSets] Failed to find descriptor set by ID: {0}", setID));
        return static_cast<uint32_t>(m_DescriptorSets[setID].size());
    }

    const DescriptorSetLayout& VulkanDescriptorSets::GetLayout(uint8_t setID) const
    {
        LU_VERIFY(!m_OriginalLayouts[setID].Descriptors.empty(), std::format("[VkDescriptorSets] Failed to find descriptor set by ID: {0}", setID));
        return m_OriginalLayouts[setID];
    }

    std::vector<DescriptorSet*> VulkanDescriptorSets::GetSets(uint8_t setID)
    {
        LU_VERIFY(!m_OriginalLayouts[setID].Descriptors.empty(), std::format("[VkDescriptorSets] Failed to find descriptor set by ID: {0}", setID));

        std::vector<DescriptorSet*> sets(m_DescriptorSets[setID].size());
        for (size_t i = 0; i < sets.size(); i++)
            sets[i] = Vk::Cast<DescriptorSet>(& m_DescriptorSets[setID][i]);

        return sets;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanDescriptorSets::CreateDescriptorSetLayout(uint8_t setID)
    {
        std::vector<VkDescriptorSetLayoutBinding> layouts;
        layouts.reserve(m_OriginalLayouts[setID].Descriptors.size());

        std::vector<VkDescriptorBindingFlags> bindingFlags;
        bindingFlags.reserve(m_OriginalLayouts[setID].Descriptors.size());

        bool bindless = false;
        for (const auto& [name, descriptor] : m_OriginalLayouts[setID].Descriptors)
        {
            LU_VERIFY((descriptor.Count != 0), "[VkDescriptorSets] Descriptor.Count == 0.");

            VkDescriptorSetLayoutBinding& layoutBinding = layouts.emplace_back();
            layoutBinding.binding = descriptor.Binding;
            layoutBinding.descriptorType = DescriptorTypeToVkDescriptorType(descriptor.Type);
            layoutBinding.descriptorCount = descriptor.Count;
            layoutBinding.stageFlags = ShaderStageToVkShaderStageFlags(descriptor.Stage);
            layoutBinding.pImmutableSamplers = nullptr; // Optional

            // Bindless checks/features
            bindingFlags.emplace_back(DescriptorBindingFlagsToVkDescriptorBindingFlags(descriptor.BindingFlags));
            bindless |= (descriptor.BindingFlags != DescriptorBindingFlags::None);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layouts.size());
        layoutInfo.pBindings = layouts.data();
        layoutInfo.flags = (bindless ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT : 0); // For bindless support

        // Add custom bindingFlags when there is a bindless descriptor found
        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedInfo = {};
        if (bindless)
        {
            extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
            extendedInfo.bindingCount = static_cast<uint32_t>(layouts.size());
            extendedInfo.pBindingFlags = bindingFlags.data();

            layoutInfo.pNext = &extendedInfo;
        }

        m_DescriptorLayouts[setID] = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateDescriptorSetLayout(VulkanContext::GetVulkanDevice().GetVkDevice(), &layoutInfo, nullptr, &m_DescriptorLayouts[setID]));
    }

    void VulkanDescriptorSets::CreateDescriptorPool(uint8_t setID, uint32_t amount)
    {
        // Note: Just for myself, the poolSizes is just the amount of elements of a certain type to able to allocate per pool
        std::vector<VkDescriptorPoolSize> poolSizes;
        poolSizes.reserve(m_OriginalLayouts[setID].UniqueTypes().size());

        const uint32_t framesInFlight = static_cast<uint32_t>(Renderer::GetRenderer(m_RendererID).GetSpecification().Buffers);
        for (const auto& type : m_OriginalLayouts[setID].UniqueTypes())
        {
            VkDescriptorPoolSize& poolSize = poolSizes.emplace_back();
            poolSize.type = DescriptorTypeToVkDescriptorType(type);
            poolSize.descriptorCount = m_OriginalLayouts[setID].AmountOf(type) * framesInFlight * amount;
        }

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = framesInFlight * amount; // A set for every frame in flight
        poolInfo.flags = (m_OriginalLayouts[setID].ContainsBindless() ? VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT : 0); // For bindless support

        m_DescriptorPools[setID] = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateDescriptorPool(VulkanContext::GetVulkanDevice().GetVkDevice(), &poolInfo, nullptr, &m_DescriptorPools[setID]));
    }

    void VulkanDescriptorSets::CreateDescriptorSets(uint8_t setID, uint32_t amount)
    {
        const uint32_t framesInFlight = static_cast<uint32_t>(Renderer::GetRenderer(m_RendererID).GetSpecification().Buffers);

        std::vector<VkDescriptorSet> descriptorSets;
        descriptorSets.resize(static_cast<size_t>(framesInFlight) * amount);

        std::vector<VkDescriptorSetLayout> layouts(static_cast<size_t>(framesInFlight) * amount, m_DescriptorLayouts[setID]);

        std::vector<uint32_t> maxBindings;
        maxBindings.reserve(static_cast<size_t>(framesInFlight) * amount);

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPools[setID];
        allocInfo.descriptorSetCount = framesInFlight * amount;
        allocInfo.pSetLayouts = layouts.data();

        bool bindless = false;
        for (const auto& [name, descriptor] : m_OriginalLayouts[setID].Descriptors)
        {
            LU_VERIFY((descriptor.Count != 0), "[VkDescriptorSets] Descriptor.Count == 0.");

            const uint32_t maxBinding = descriptor.Count - 1;
            for (uint32_t i = 0; i < framesInFlight; i++)
                maxBindings.push_back(maxBinding);

            bindless |= (descriptor.BindingFlags != DescriptorBindingFlags::None);
        }

        // Set max bindings when there is a bindless descriptor found
        VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countInfo = {};
        if (bindless)
        {
            countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
            countInfo.descriptorSetCount = framesInFlight * amount;
            countInfo.pDescriptorCounts = maxBindings.data();

            allocInfo.pNext = &countInfo;
        }

        VK_VERIFY(vkAllocateDescriptorSets(VulkanContext::GetVulkanDevice().GetVkDevice(), &allocInfo, descriptorSets.data()));
        ConvertToVulkanDescriptorSets(setID, amount, descriptorSets);
    }

    void VulkanDescriptorSets::ConvertToVulkanDescriptorSets(uint8_t setID, uint32_t amount, std::vector<VkDescriptorSet>& sets)
    {
        m_DescriptorSets[setID].resize(static_cast<size_t>(amount));

        const uint32_t framesInFlight = static_cast<uint32_t>(Renderer::GetRenderer(m_RendererID).GetSpecification().Buffers);

        size_t index = 0;
        for (uint32_t i = 0; i < amount; i++)
        {
            std::vector<VkDescriptorSet> setCombo = { };

            for (uint32_t j = 0; j < framesInFlight; j++)
                setCombo.push_back(sets[index + j]);

            m_DescriptorSets[setID][i].Init(m_RendererID, setID, setCombo);
            index += framesInFlight;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Convert functions 
    ////////////////////////////////////////////////////////////////////////////////////
    DescriptorType VkDescriptorTypeToDescriptorType(VkDescriptorType type)
    {
        switch (type)
        {
        case VK_DESCRIPTOR_TYPE_SAMPLER:                                        return DescriptorType::Sampler;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:                         return DescriptorType::CombinedImageSampler;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:                                  return DescriptorType::SampledImage;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:                                  return DescriptorType::StorageImage;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:                           return DescriptorType::UniformTexelBuffer;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:                           return DescriptorType::StorageTexelBuffer;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:                                 return DescriptorType::UniformBuffer;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:                                 return DescriptorType::StorageBuffer;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:                         return DescriptorType::DynamicUniformBuffer;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:                         return DescriptorType::DynamicStorageBuffer;
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:                               return DescriptorType::InputAttachment;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:                     return DescriptorType::AccelerationStructureKHR;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:                      return DescriptorType::AccelerationStructureNV;
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM:                       return DescriptorType::SampleWeightImageQCOM;
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM:                         return DescriptorType::BlockMatchImageQCOM;
        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:                                    return DescriptorType::MutableEXT;
        
        default:
            LU_ASSERT(false, "[VkDescriptorSet] Invalid descriptor type passed in.");
            break;
        }

        return DescriptorType::None;
    }

    VkDescriptorType DescriptorTypeToVkDescriptorType(DescriptorType type)
    {
        switch (type)
        {
        case DescriptorType::Sampler:                                           return VK_DESCRIPTOR_TYPE_SAMPLER;
        case DescriptorType::CombinedImageSampler:                              return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DescriptorType::SampledImage:                                      return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescriptorType::StorageImage:                                      return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case DescriptorType::UniformTexelBuffer:                                return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case DescriptorType::StorageTexelBuffer:                                return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        case DescriptorType::UniformBuffer:                                     return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::StorageBuffer:                                     return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::DynamicUniformBuffer:                              return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        case DescriptorType::DynamicStorageBuffer:                              return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        case DescriptorType::InputAttachment:                                   return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        case DescriptorType::AccelerationStructureKHR:                          return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        case DescriptorType::AccelerationStructureNV:                           return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
        case DescriptorType::SampleWeightImageQCOM:                             return VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM;
        case DescriptorType::BlockMatchImageQCOM:                               return VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM;
        case DescriptorType::MutableEXT:                                        return VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
        
        default: 
            LU_ASSERT(false, "[VkDescriptorSet] Invalid descriptor type passed in.");
            break;
        }

        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

    DescriptorBindingFlags VkDescriptorBindingFlagsToDescriptorBindingFlags(VkDescriptorBindingFlags flags)
    {
        DescriptorBindingFlags result = DescriptorBindingFlags::None;

        if (flags & VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)                result |= DescriptorBindingFlags::UpdateAfterBind;
        if (flags & VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT)      result |= DescriptorBindingFlags::UpdateUnusedWhilePending;
        if (flags & VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT)                  result |= DescriptorBindingFlags::PartiallyBound;
        if (flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT)        result |= DescriptorBindingFlags::VariableDescriptorCount;

        return result;
    }

    VkDescriptorBindingFlags DescriptorBindingFlagsToVkDescriptorBindingFlags(DescriptorBindingFlags flags)
    {
        VkDescriptorBindingFlags result = 0;

        if (flags & DescriptorBindingFlags::UpdateAfterBind)                    result |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
        if (flags & DescriptorBindingFlags::UpdateUnusedWhilePending)           result |= VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
        if (flags & DescriptorBindingFlags::PartiallyBound)                     result |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
        if (flags & DescriptorBindingFlags::VariableDescriptorCount)            result |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

        return result;
    }


}