#pragma once

#include <cstdint>
#include <utility>
#include <array>
#include <tuple>

#include "Lunar/IO/Print.hpp"

#include "Lunar/Enum/Name.hpp"

#include "Lunar/Utils/Preprocessor.hpp"

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace Lunar
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Configuration
    ////////////////////////////////////////////////////////////////////////////////////
    inline constinit const std::tuple<uint8_t, uint8_t> g_VkVersion = { 1, 3 };
	
    #if !defined(LU_CONFIG_DIST)
        inline constexpr const bool g_VkValidation = true;
    #else
        inline constexpr const bool g_VkValidation = false;
    #endif

    inline constexpr auto g_VkRequestedValidationLayers = std::to_array<const char*>({ 
        "VK_LAYER_KHRONOS_validation" 
    });
    
    inline constexpr auto g_VkRequestedDeviceExtensions = std::to_array<const char*>({
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
            
        #if defined(LU_PLATFORM_MACOS)
        "VK_KHR_portability_subset"
        #endif
    });
    
    inline constexpr VkPhysicalDeviceFeatures g_VkRequestedDeviceFeatures = {
        .fillModeNonSolid = VK_TRUE,
        // .wideLines = VK_TRUE, // Note: Disabled for compatibility reasons
        .samplerAnisotropy = VK_TRUE,
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Helper
    ////////////////////////////////////////////////////////////////////////////////////
    #define VK_VERIFY_IMPL(expr, str, num)                                                                          \
        VkResult result##num = expr;                                                                                \
        if (result##randomizedNr != VK_SUCCESS)                                                                     \
        LU_LOG_ERROR("Expression {0} failed with error code: {1}", str, ::Lunar::Enum::Name(result##randomizedNr))

    #if !defined(LU_CONFIG_DIST)
        #define VK_VERIFY(expr) VK_VERIFY_IMPL((expr), #expr, LU_EXPAND_MACRO(__COUNTER__))
    #else
        #define VK_VERIFY(expr) expr
    #endif

}