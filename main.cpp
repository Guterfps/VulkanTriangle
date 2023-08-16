
//#include <vulkan/vulkan.h> // vulkan header

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream> // for errors
#include <stdexcept> // std::exception
#include <cstdlib> // EXIT_FAILURE, EXIT_SUCCESS
#include <vector> // std::vector
#include <algorithm> // std::find
#include <cstring> // strcmp
#include <map> // std::map
#include <set> // std::set
#include <optional> // std::optional
#include <limits> // std::numeric_limits
#include <algorithm> // std::clamp

class TriangleApp
{
public:
    void Run();

private:
    GLFWwindow *m_window{nullptr};
    VkInstance m_instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkDevice m_device{VK_NULL_HANDLE};
    VkQueue m_graphicsQueue{VK_NULL_HANDLE};
    VkQueue m_presentQueue{VK_NULL_HANDLE};
    VkSwapchainKHR m_swapChain{VK_NULL_HANDLE};
    std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;

    void InitWindow();
    void InitVulkan();
    void CreateInstance();
    void SetUpDebugMessenger();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapChain();
    void MainLoop();
    void Cleanup();

    static bool CheckExstensions(const char **glfwExtensions, 
                                    uint32_t glfwExtensionCount);
    static bool CheckValidationLayerSupport();
    static std::vector<const char*> 
    GetRequiredExtensions(const char **glfwExtensions, 
                            uint32_t glfwExtensionCount);
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData);
    static void PopulateDebugMessengerCreateInfo(
                VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    bool IsDeviceSuitable(VkPhysicalDevice device);
    static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    int RateDevice(VkPhysicalDevice device);
    
    struct QueueFamilyIndices;
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    struct SwapChainSupportDetails;
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
    static VkSurfaceFormatKHR ChooseSwapSurfaceFormat
    (const std::vector<VkSurfaceFormatKHR>& availableFormats);
    static VkPresentModeKHR ChooseSwapPresentMode
    (const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);


    static constexpr uint32_t WIDTH = 800;
    static constexpr uint32_t HEIGHT = 600;
    static const std::vector<const char*> s_validationLayers;
    static const std::vector<const char*> s_deviceExtensions;

    #ifdef NDEBUG
        static constexpr bool s_enableValidationLayers = false;
    #else
        static constexpr bool s_enableValidationLayers = true;
    #endif

    struct QueueFamilyIndices
    {
        std::optional<uint32_t>  m_graphicsFamily;
        std::optional<uint32_t>  m_presentFamily;

        bool IsComplete()
        {
            return (m_graphicsFamily.has_value() &&
                    m_presentFamily.has_value());
        }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR m_capabilities;
        std::vector<VkSurfaceFormatKHR> m_formats;
        std::vector<VkPresentModeKHR> m_presentModes;
    };
};

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger); 
void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator);


int main()
{
    TriangleApp app;

    try
    {
        app.Run();
    } catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}

const std::vector<const char*> TriangleApp::s_validationLayers =
                                {"VK_LAYER_KHRONOS_validation"};
const std::vector<const char*> TriangleApp::s_deviceExtensions =
                                {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

inline void TriangleApp::Run()
{
    InitWindow();
    InitVulkan();
    MainLoop();
    Cleanup();
}

void TriangleApp::InitWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void TriangleApp::InitVulkan()
{
    CreateInstance();
    SetUpDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();

}

void TriangleApp::SetUpDebugMessenger()
{
    if (s_enableValidationLayers)
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(m_instance, &createInfo, 
            nullptr, &m_debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger");
        }
    }
}

void TriangleApp::CreateInstance()
{
    if (s_enableValidationLayers && !CheckValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "Vulkan Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = 
    glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    auto extensions = GetRequiredExtensions(glfwExtensions, glfwExtensionCount);
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (s_enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>
                                        (s_validationLayers.size());
        createInfo.ppEnabledLayerNames = s_validationLayers.data();

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (!CheckExstensions(glfwExtensions, glfwExtensionCount))
    {
        throw std::runtime_error("not all extensions are supported");
    }

    if (VK_SUCCESS != vkCreateInstance(&createInfo, nullptr, &m_instance))
    {
        throw std::runtime_error("failed to create instance");
    }

}

void TriangleApp::CreateSurface()
{
    if (VK_SUCCESS != glfwCreateWindowSurface(m_instance, m_window, 
                                                nullptr, &m_surface))
    {
        throw std::runtime_error("failed to create window surface");
    }
}

bool TriangleApp::CheckExstensions(const char **glfwExtensions, 
                                    uint32_t glfwExtensionCount)
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> supportedExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, 
                                            supportedExtensions.data());
    std::cout << "Available extensions " << extensionCount << ":" << std::endl;

    for (const auto& extension : supportedExtensions)
    {
        std::cout << '\t' << extension.extensionName << '\n';
    }

    for (uint32_t i = 0; i < glfwExtensionCount; ++i)
    {
        if (std::find_if(supportedExtensions.begin(), supportedExtensions.end(),
            [glfwExtensions, i](const auto& extension)
            {
                return (0 == std::strcmp(extension.extensionName, 
                                        glfwExtensions[i]));
            })  == supportedExtensions.end())
        {
            return false;
        }
    }

    std::cout << "All " << glfwExtensionCount << 
    " GLFW extensions are supported" << std::endl;

    return true;
}

void TriangleApp::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    if (0 == deviceCount)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    std::multimap<int, VkPhysicalDevice> deviceMap;

    for (const auto& device : devices)
    {
        int score = RateDevice(device);
        deviceMap.emplace(score, device);
    }

    if (deviceMap.rbegin()->first > 0)
    {
        m_physicalDevice = deviceMap.rbegin()->second;
    }
    else
    {
        throw std::runtime_error("failed to find a suitable GPU");
    }
}

void TriangleApp::CreateLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = 
    {indices.m_graphicsFamily.value(), indices.m_presentFamily.value()};
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>
                                        (queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>
                                        (s_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = s_deviceExtensions.data();

    if (s_enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>
                                        (s_validationLayers.size());
        createInfo.ppEnabledLayerNames = s_validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (VK_SUCCESS != vkCreateDevice(m_physicalDevice, &createInfo, 
                                    nullptr, &m_device))
    {
        throw std::runtime_error("failed to create logical device");
    }

    vkGetDeviceQueue(m_device, indices.m_graphicsFamily.value(), 
                        0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.m_presentFamily.value(), 
                        0, &m_presentQueue);
}

void TriangleApp::CreateSwapChain()
{
    SwapChainSupportDetails swapChainSupport = 
                        QuerySwapChainSupport(m_physicalDevice);
    VkSurfaceFormatKHR surfaceFormat = 
                        ChooseSwapSurfaceFormat(swapChainSupport.m_formats);
    VkPresentModeKHR presentMode = 
                        ChooseSwapPresentMode(swapChainSupport.m_presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.m_capabilities);
    uint32_t imageCount = swapChainSupport.m_capabilities.minImageCount + 1;
    if ((swapChainSupport.m_capabilities.maxImageCount > 0) && 
        (imageCount > swapChainSupport.m_capabilities.maxImageCount))
    {
        imageCount = swapChainSupport.m_capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
    uint32_t queueFamilyIndices[] = 
    {indices.m_graphicsFamily.value(), indices.m_presentFamily.value()};

    if (indices.m_graphicsFamily != indices.m_presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    createInfo.preTransform = swapChainSupport.m_capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (VK_SUCCESS != vkCreateSwapchainKHR(m_device, &createInfo, 
                                            nullptr, &m_swapChain))
    {
        throw std::runtime_error("failed to create swap chain");
    }

    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount,
                            m_swapChainImages.data());
    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
}

void TriangleApp::MainLoop()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
    }
}

void TriangleApp::Cleanup()
{
    vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
    vkDestroyDevice(m_device, nullptr);
    
    if (s_enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
    
    glfwDestroyWindow(m_window);

    glfwTerminate();
}

bool TriangleApp::CheckValidationLayerSupport()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto& layer : s_validationLayers)
    {
        if (std::find_if(availableLayers.begin(), availableLayers.end(),
            [layer](const auto& availableLayer)
            {
                return (0 == std::strcmp(availableLayer.layerName, layer));
            }) == availableLayers.end())
        {
            return false;
        }
    }

    return true;
}

inline std::vector<const char*> TriangleApp::
GetRequiredExtensions(const char **glfwExtensions, uint32_t glfwExtensionCount)
{
    std::vector<const char*> extensions(glfwExtensions, 
                                        glfwExtensions + glfwExtensionCount);
    if (s_enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}


void TriangleApp::PopulateDebugMessengerCreateInfo(
                    VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = 
    VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = 
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | 
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = &DebugCallback;
    createInfo.pUserData = nullptr;
}

bool TriangleApp::IsDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = FindQueueFamilies(device);
    bool extensionsSupported = CheckDeviceExtensionSupport(device);
    bool swapChainAdequate = false;

    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate = (!swapChainSupport.m_formats.empty() && 
                             !swapChainSupport.m_presentModes.empty());
    }

    return (indices.IsComplete() && swapChainAdequate);
}

bool TriangleApp::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, 
                                        &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, 
                                &extensionCount, availableExtensions.data());
    std::set<std::string> requiredExtensions(s_deviceExtensions.begin(),
                                            s_deviceExtensions.end());
    
    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return (requiredExtensions.empty());
}

int TriangleApp::RateDevice(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    int score = 0;

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        score += 1000;
    }

    score += deviceProperties.limits.maxImageDimension2D;

    if ((deviceFeatures.geometryShader == VK_FALSE) || 
        (false == IsDeviceSuitable(device)))
    {
        return 0;
    }
    
    QueueFamilyIndices indices = FindQueueFamilies(device);
    if (indices.m_graphicsFamily.value() == indices.m_presentFamily.value())
    {
        score += 100;
    }

    return score;
}

TriangleApp::QueueFamilyIndices 
TriangleApp::FindQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 
                                            queueFamilies.data());
    for (uint32_t i = 0; (i < queueFamilyCount) && !indices.IsComplete(); ++i)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.m_graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, 
                                            m_surface, &presentSupport);
        if (presentSupport)
        {
            indices.m_presentFamily = i;
        }
    }

    return indices;
}

TriangleApp::SwapChainSupportDetails 
TriangleApp::QuerySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, 
                                                &details.m_capabilities);
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
    if (0 != formatCount)
    {
        details.m_formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount,
                                             details.m_formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, 
                                                &presentModeCount, nullptr);
    if (0 != presentModeCount)
    {
        details.m_presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, 
                        &presentModeCount, details.m_presentModes.data());
    }

    return details;
}

inline VkSurfaceFormatKHR TriangleApp::ChooseSwapSurfaceFormat
    (const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if ((VK_FORMAT_B8G8R8A8_SRGB == availableFormat.format) &&
            (VK_COLOR_SPACE_SRGB_NONLINEAR_KHR == availableFormat.colorSpace))
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

inline VkPresentModeKHR TriangleApp::ChooseSwapPresentMode
    (const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availPresentMode : availablePresentModes)
    {
        if (VK_PRESENT_MODE_MAILBOX_KHR == availPresentMode)
        {
            return availPresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

inline VkExtent2D TriangleApp::ChooseSwapExtent
    (const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_window, &width, &height);
        VkExtent2D actualExtent = 
        {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, 
        capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, 
        capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL TriangleApp::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
        const VkAllocationCallbacks *pAllocator,
        VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = PFN_vkCreateDebugUtilsMessengerEXT(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

    
    return ((nullptr != func) ? 
            func(instance, pCreateInfo, pAllocator, pDebugMessenger) : 
            VK_ERROR_EXTENSION_NOT_PRESENT);
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator)
{
    auto func = PFN_vkDestroyDebugUtilsMessengerEXT(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (nullptr != func)
    {
        func(instance, debugMessenger, pAllocator);
    }
}