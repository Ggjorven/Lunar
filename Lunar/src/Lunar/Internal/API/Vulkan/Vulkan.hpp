#pragma once

#include <cstdint>
#include <utility>
#include <array>
#include <tuple>

#include "Lunar/Enum/Name.hpp"

#include "Lunar/Internal/IO/Print.hpp"
#include "Lunar/Internal/Utils/Settings.hpp"

#include "Lunar/Internal/Renderer/RendererSpec.hpp"

#include "Lunar/Internal/Utils/Preprocessor.hpp"

#if defined(LU_COMPILER_GCC)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

    #pragma GCC diagnostic pop
#elif !defined(LU_PLATFORM_APPLE)
    #pragma warning(push, 0)

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

    #pragma warning(pop)
#endif

////////////////////////////////////////////////////////////////////////////////////
// Helper
////////////////////////////////////////////////////////////////////////////////////
template<>
struct Lunar::Enum::Range<VkResult>
{
public:
    inline static constexpr int32_t Min = -11; // VK_ERROR_FORMAT_NOT_SUPPORTED
    inline static constexpr int32_t Max = 5;
};

namespace Lunar::Internal
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
        "VK_LAYER_KHRONOS_validation", "VK_LAYER_KHRONOS_synchronization2",
        // "VK_EXT_debug_utils"
    });

    inline constexpr auto g_VkRequestedDeviceExtensions = std::to_array<const char*>({
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,

        #if defined(LU_PLATFORM_MACOS)
        "VK_KHR_portability_subset"
        #endif
    });

    inline constexpr VkPhysicalDeviceFeatures g_VkRequestedDeviceFeatures = {
        .robustBufferAccess = VK_FALSE,
        .fullDrawIndexUint32 = VK_FALSE,
        .imageCubeArray = VK_FALSE,
        .independentBlend = VK_FALSE,
        .geometryShader = VK_FALSE,
        .tessellationShader = VK_FALSE,
        .sampleRateShading = VK_FALSE,
        .dualSrcBlend = VK_FALSE,
        .logicOp = VK_FALSE,
        .multiDrawIndirect = VK_FALSE,
        .drawIndirectFirstInstance = VK_FALSE,
        .depthClamp = VK_FALSE,
        .depthBiasClamp = VK_FALSE,
        .fillModeNonSolid = VK_TRUE, // Needed
        .depthBounds = VK_FALSE,
		.wideLines = VK_FALSE, // Note: Disabled for compatibility reasons
        .largePoints = VK_FALSE,
        .alphaToOne = VK_FALSE,
        .multiViewport = VK_FALSE,
		.samplerAnisotropy = VK_TRUE, // Needed
        .textureCompressionETC2 = VK_FALSE,
        .textureCompressionASTC_LDR = VK_FALSE,
        .textureCompressionBC = VK_FALSE,
        .occlusionQueryPrecise = VK_FALSE,
        .pipelineStatisticsQuery = VK_FALSE,
        .vertexPipelineStoresAndAtomics = VK_FALSE,
        .fragmentStoresAndAtomics = VK_FALSE,
        .shaderTessellationAndGeometryPointSize = VK_FALSE,
        .shaderImageGatherExtended = VK_FALSE,
        .shaderStorageImageExtendedFormats = VK_FALSE,
        .shaderStorageImageMultisample = VK_FALSE,
        .shaderStorageImageReadWithoutFormat = VK_FALSE,
        .shaderStorageImageWriteWithoutFormat = VK_FALSE,
        .shaderUniformBufferArrayDynamicIndexing = VK_FALSE,
        .shaderSampledImageArrayDynamicIndexing = VK_FALSE,
        .shaderStorageBufferArrayDynamicIndexing = VK_FALSE,
        .shaderStorageImageArrayDynamicIndexing = VK_FALSE,
        .shaderClipDistance = VK_FALSE,
        .shaderCullDistance = VK_FALSE,
        .shaderFloat64 = VK_FALSE,
        .shaderInt64 = VK_FALSE,
        .shaderInt16 = VK_FALSE,
        .shaderResourceResidency = VK_FALSE,
        .shaderResourceMinLod = VK_FALSE,
        .sparseBinding = VK_FALSE,
        .sparseResidencyBuffer = VK_FALSE,
        .sparseResidencyImage2D = VK_FALSE,
        .sparseResidencyImage3D = VK_FALSE,
        .sparseResidency2Samples = VK_FALSE,
        .sparseResidency4Samples = VK_FALSE,
        .sparseResidency8Samples = VK_FALSE,
        .sparseResidency16Samples = VK_FALSE,
        .sparseResidencyAliased = VK_FALSE,
        .variableMultisampleRate = VK_FALSE,
        .inheritedQueries = VK_FALSE
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Helper
    ////////////////////////////////////////////////////////////////////////////////////
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
        #error Lunar Vulkan: Unsupported platform...
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

        template<typename T, typename T2>
        inline T* Cast(T2* from)
        {
            // Note: This is temporary (or at least supposed to be)
            return reinterpret_cast<T*>(from);
        }

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

        inline VkCommandBuffer GetVkCommandBuffer() const { return m_CommandBuffer; }

    private:
        const RendererID m_Renderer;
        VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
    };

}