#include "lupch.h"
#include "VulkanContext.hpp"

#include "Lunar/IO/Print.hpp"

#include "Lunar/API/Vulkan/VulkanAllocator.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Lunar
{
    
    ////////////////////////////////////////////////////////////////////////////////////
    // Constructors & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanContext::VulkanContext(std::vector<void*>& windows)
    {
        s_Instance = this;
        LU_ASSERT(!windows.empty(), "[VulkanContext] No window was attached.");

        InitInstance();
        InitDevices(windows.front());

        VulkanAllocator::Init();
    }

    VulkanContext::~VulkanContext()
    {
        //Renderer::FreeObjects();
        VulkanAllocator::Destroy();

        m_PhysicalDevice.Reset();
        m_Device.Reset();

        if constexpr (g_VkValidation) 
        {
            if (m_DebugMessenger)
                Vk::DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        }

        vkDestroyInstance(m_Instance, nullptr);
        s_Instance = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanContext::InitInstance()
    {
        ///////////////////////////////////////////////////////////
		// Instance Creation
		///////////////////////////////////////////////////////////
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Lunar Application";
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, std::get<0>(g_VkVersion), std::get<1>(g_VkVersion), 0);
		appInfo.pEngineName = "Lunar Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, std::get<0>(g_VkVersion), std::get<1>(g_VkVersion), 0);
        appInfo.apiVersion = VK_MAKE_API_VERSION(0, std::get<0>(g_VkVersion), std::get<1>(g_VkVersion), 0);

        // Check for validation layer support
        bool validationSupport = Vk::ValidationLayersSupported();
        if constexpr (g_VkValidation)
        {
            if (!validationSupport)
                LU_LOG_WARN("[VulkanContext] Requested validation layers, but no support found.");
        }

		std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_SURFACE_TYPE_NAME };
		if constexpr (g_VkValidation)
		{
            if (validationSupport)
            {
                instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
                instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            }
		}

        #if defined(LU_PLATFORM_MACOS)
            instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        #endif

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
        #if defined(LU_PLATFORM_MACOS)
			createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
		#endif
		createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
		createInfo.ppEnabledExtensionNames = instanceExtensions.data();

		if constexpr (g_VkValidation)
		{
            if (validationSupport)
            {
                createInfo.enabledLayerCount = static_cast<uint32_t>(g_VkRequestedValidationLayers.size());
                createInfo.ppEnabledLayerNames = g_VkRequestedValidationLayers.data();
            }
            else
            {
			    createInfo.enabledLayerCount = 0;
            }
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		// Note: Setup the debug messenger also for the create instance
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = &Vk::VulkanDebugCallback;

		if constexpr (g_VkValidation)
		{
            if (validationSupport)
            {
			    createInfo.enabledLayerCount = static_cast<uint32_t>(g_VkRequestedValidationLayers.size());
                createInfo.ppEnabledLayerNames = g_VkRequestedValidationLayers.data();

			    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
            }
            else
            {
                createInfo.enabledLayerCount = 0;
                createInfo.pNext = nullptr;
            }
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		VK_VERIFY(vkCreateInstance(&createInfo, nullptr, &m_Instance));

		///////////////////////////////////////////////////////////
		// Debugger Creation
		///////////////////////////////////////////////////////////
		if constexpr (g_VkValidation)
		{
            if (validationSupport)
            {
                VK_VERIFY(Vk::CreateDebugUtilsMessengerEXT(m_Instance, &debugCreateInfo, nullptr, &m_DebugMessenger));
            }
        }
    }

    void VulkanContext::InitDevices(void* window)
    {
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        #if defined(LU_PLATFORM_DESKTOP)
        VK_VERIFY(glfwCreateWindowSurface(m_Instance, static_cast<GLFWwindow*>(window), nullptr, &surface));
        #endif

        m_PhysicalDevice = VulkanPhysicalDevice::Select(surface);
        m_Device = VulkanDevice::Create(surface, m_PhysicalDevice);

        vkDestroySurfaceKHR(m_Instance, surface, nullptr);
    }

}