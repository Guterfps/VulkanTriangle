
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
#include <fstream> // std::ifstream

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
    std::vector<VkImageView> m_swapChainImageViews;
    VkRenderPass m_renderPass{VK_NULL_HANDLE};
    VkPipelineLayout m_pipelineLayout{VK_NULL_HANDLE};
    VkPipeline m_graphicsPipeline{VK_NULL_HANDLE};
    std::vector<VkFramebuffer> m_swapChainFramebuffers;
    VkCommandPool m_commandPool{VK_NULL_HANDLE};
    VkCommandBuffer m_commandBuffer{VK_NULL_HANDLE};
    VkSemaphore m_imageAvailableSemaphore{VK_NULL_HANDLE};
    VkSemaphore m_renderFinishedSemaphore{VK_NULL_HANDLE};
    VkFence m_inFlightFence{VK_NULL_HANDLE};

    void InitWindow();
    void InitVulkan();
    void CreateInstance();
    void SetUpDebugMessenger();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSwapChain();
    void CreateImageViews();
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffer();
    void CreateSyncObjects();
    void MainLoop();
    void DrawFrame();
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
    static std::vector<char> ReadFile(const std::string& filename);
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, 
                                    uint32_t imageIndex);


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
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandPool();
    CreateCommandBuffer();
    CreateSyncObjects();

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

void TriangleApp::CreateImageViews()
{
    m_swapChainImageViews.resize(m_swapChainImages.size());

    for (std::size_t i = 0; i < m_swapChainImages.size(); ++i)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (VK_SUCCESS != vkCreateImageView(m_device, &createInfo, 
                                nullptr, &m_swapChainImageViews[i]))
        {
            throw std::runtime_error("failed to create image views");
        }
    }
}

void TriangleApp::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (VK_SUCCESS != vkCreateRenderPass(m_device, &renderPassInfo, 
                                        nullptr, &m_renderPass))
    {
        throw std::runtime_error("failed to create render pass");
    }
}


void TriangleApp::CreateGraphicsPipeline()
{
    auto vertShaderCode = ReadFile("shaders/vert.spv");
    auto fragShaderCode = ReadFile("shaders/frag.spv");

    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertStageInfo{};
    vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStageInfo.module = vertShaderModule;
    vertStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragStageInfo{};
    fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStageInfo.module = fragShaderModule;
    fragStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = 
    {vertStageInfo, fragStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    std::vector<VkDynamicState> dynamicStates = 
    {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateInfo.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
    VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (VK_SUCCESS != vkCreatePipelineLayout(m_device, &pipelineLayoutInfo,
                                        nullptr, &m_pipelineLayout))
    {
        throw std::runtime_error("failed to create pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicStateInfo;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (VK_SUCCESS != vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE,
                1, &pipelineInfo, nullptr, &m_graphicsPipeline))
    {
        throw std::runtime_error("failed to create graphics pipeline");
    }

    vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
}

void TriangleApp::CreateFramebuffers()
{
    m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

    for (std::size_t i = 0; i < m_swapChainImageViews.size(); ++i)
    {
        VkImageView attachments[] = {m_swapChainImageViews[i]};
        
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapChainExtent.width;
        framebufferInfo.height = m_swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (VK_SUCCESS != vkCreateFramebuffer(m_device, &framebufferInfo,
                                nullptr, &m_swapChainFramebuffers[i]))
        {
            throw std::runtime_error("failed to create framebuffer");
        }
    }
}

void TriangleApp::CreateCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.m_graphicsFamily.value();

    if (VK_SUCCESS != vkCreateCommandPool(m_device, &poolInfo, 
                                    nullptr, &m_commandPool))
    {
        throw std::runtime_error("failed to create command pool");
    }
}

void TriangleApp::CreateCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (VK_SUCCESS != vkAllocateCommandBuffers(m_device, &allocInfo,
                                                &m_commandBuffer))
    {
        throw std::runtime_error("failed to allocate command buffers");
    }
}

void TriangleApp::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    if ((VK_SUCCESS != vkCreateSemaphore(m_device, &semaphoreInfo, 
                            nullptr, &m_imageAvailableSemaphore)) || 
        (VK_SUCCESS != vkCreateSemaphore(m_device, &semaphoreInfo,
                            nullptr, &m_renderFinishedSemaphore)) ||
        (VK_SUCCESS != vkCreateFence(m_device, &fenceInfo,
                             nullptr, &m_inFlightFence)))
    {
        throw std::runtime_error("failed to create sync objects");
    }
}
void TriangleApp::MainLoop()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        DrawFrame();
    }

    vkDeviceWaitIdle(m_device);
}

void TriangleApp::DrawFrame()
{
    vkWaitForFences(m_device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &m_inFlightFence);
    
    uint32_t imageIndex = 0;
    vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, 
                    m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    
    vkResetCommandBuffer(m_commandBuffer, 0);
    RecordCommandBuffer(m_commandBuffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = 
    {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;

    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (VK_SUCCESS != vkQueueSubmit(m_graphicsQueue, 1, &submitInfo,
                                    m_inFlightFence))
    {
        throw std::runtime_error("failed to submit draw command buffer");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapChains[] = {m_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(m_presentQueue, &presentInfo);

}

void TriangleApp::Cleanup()
{
    vkDestroyFence(m_device, m_inFlightFence, nullptr);
    vkDestroySemaphore(m_device, m_renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(m_device, m_imageAvailableSemaphore, nullptr);
    
    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    
    for (auto framebuffer : m_swapChainFramebuffers)
    {
        vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }
    
    vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    
    for (VkImageView imageView : m_swapChainImageViews)
    {
        vkDestroyImageView(m_device, imageView, nullptr);
    }
    
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

inline std::vector<char> TriangleApp::ReadFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file");
    }

    std::size_t fileSize = static_cast<std::size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

inline VkShaderModule TriangleApp::CreateShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (VK_SUCCESS != vkCreateShaderModule(m_device, &createInfo, 
                                            nullptr, &shaderModule))
    {
        throw std::runtime_error("failed to create shader module");
    }

    return shaderModule;
}

void TriangleApp::RecordCommandBuffer(VkCommandBuffer commandBuffer, 
                                        uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (VK_SUCCESS != vkBeginCommandBuffer(commandBuffer, &beginInfo))
    {
        throw std::runtime_error("failed to begin recording command buffer");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, 
                            VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
                            m_graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapChainExtent.width);
    viewport.height = static_cast<float>(m_swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapChainExtent;
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    if (VK_SUCCESS != vkEndCommandBuffer(commandBuffer))
    {
        throw std::runtime_error("failed to record command buffer");
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