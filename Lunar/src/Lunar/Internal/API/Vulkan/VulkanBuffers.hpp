#pragma once

#include "Lunar/Internal/API/Vulkan/Vulkan.hpp"

#include "Lunar/Internal/Renderer/RendererSpec.hpp"
#include "Lunar/Internal/Renderer/BuffersSpec.hpp"

namespace Lunar::Internal
{

    class VulkanSwapChain;
    class VulkanDescriptorSet;

    ////////////////////////////////////////////////////////////////////////////////////
    // Convert functions
    ////////////////////////////////////////////////////////////////////////////////////
    VertexInputRate VkVertexInputRateToVertexInputRate(VkVertexInputRate input);
    VkVertexInputRate VertexInputRateToVkVertexInputRate(VertexInputRate input);
    DataType VkFormatToDataType(VkFormat format);
    VkFormat DataTypeToVkFormat(DataType dataType);

    ////////////////////////////////////////////////////////////////////////////////////
    // Vulkan // TODO: Buffers
    ////////////////////////////////////////////////////////////////////////////////////
    

}