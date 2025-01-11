#pragma once

#include <cstdint>
#include <utility>
#include <array>
#include <tuple>

#include "Lunar/IO/Print.hpp"

#include "Lunar/Enum/Name.hpp"

#include "Lunar/Renderer/Renderer.hpp"

#include "Lunar/Utils/Preprocessor.hpp"

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

namespace Lunar
{

    class VulkanRenderer;

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
    template<>
    struct Enum::Range<VkResult>
    {
    public:
        inline static constexpr int32_t Min = -11; // VK_ERROR_FORMAT_NOT_SUPPORTED
        inline static constexpr int32_t Max = 0;
    };

    #define VK_VERIFY_IMPL2(expr, str, num)                                                                         \
        VkResult result##num = expr;                                                                                \
        if (result##num != VK_SUCCESS)                                                                              \
        LU_LOG_ERROR("Expression '{0}' failed with error code: {1}", str, ::Lunar::Enum::Name(result##num))
    #define VK_VERIFY_IMPL(expr, str, num) VK_VERIFY_IMPL2(expr, str, num)

    #if !defined(LU_CONFIG_DIST)
        #define VK_VERIFY(expr) VK_VERIFY_IMPL((expr), #expr, __COUNTER__)
    #else
        #define VK_VERIFY(expr) expr
    #endif

    #if defined(LU_PLATFORM_WINDOWS)
        #define VK_KHR_SURFACE_TYPE_NAME "VK_KHR_win32_surface"
    #elif defined(LU_PLATFORM_LINUX)
        #define VK_KHR_SURFACE_TYPE_NAME "VK_KHR_xcb_surface"
    #elif defined(LU_PLATFORM_MACOS)
        #define VK_KHR_SURFACE_TYPE_NAME "VK_EXT_metal_surface"
    #else
        #error Unsupported platform
    #endif

    ////////////////////////////////////////////////////////////////////////////////////
    // Function implementations
    ////////////////////////////////////////////////////////////////////////////////////
    namespace Vk
    {

        VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
        void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

        VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

        bool ValidationLayersSupported();

    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Helper structs
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanCommand
    {
    public:
        VulkanCommand(const RendererID renderer, bool start);
        ~VulkanCommand();

        void Begin();
        void End();
        void Submit();
        void EndAndSubmit();

        inline const VkCommandBuffer GetVkCommandBuffer() const { return m_CommandBuffer; }

    private:
        const RendererID m_Renderer;
        VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
    };

}