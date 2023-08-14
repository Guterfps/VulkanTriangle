
//#include <vulkan/vulkan.h> // vulkan header

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream> // for errors
#include <stdexcept> // std::exception
#include <cstdlib> // EXIT_FAILURE, EXIT_SUCCESS
#include <vector> // std::vector
#include <algorithm> // std::find
#include <cstring> // strcmp

class TriangleApp
{
public:
    void Run();

private:
    GLFWwindow *m_window{nullptr};
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;

    void InitWindow();
    void InitVulkan();
    void SetUpDebugMessenger();
    void CreateInstance();
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

    static constexpr uint32_t WIDTH = 800;
    static constexpr uint32_t HEIGHT = 600;
    static const std::vector<const char*> s_validationLayers;
    #ifdef NDEBUG
        static constexpr bool s_enableValidationLayers = false;
    #else
        static constexpr bool s_enableValidationLayers = true;
    #endif
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

void TriangleApp::MainLoop()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
    }
}

void TriangleApp::Cleanup()
{
    if (s_enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    }

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

VKAPI_ATTR VkBool32 VKAPI_CALL TriangleApp::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
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