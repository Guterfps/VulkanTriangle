
//#include <vulkan/vulkan.h> // vulkan header

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream> // for errors
#include <stdexcept> // std::exception
#include <cstdlib> // EXIT_FAILURE, EXIT_SUCCESS

class TriangleApp
{
public:
    void Run();

private:
    GLFWwindow *m_window;

    void InitWindow();
    void InitVulkan();
    void MainLoop();
    void Cleanup();

    static constexpr uint32_t WIDTH = 800;
    static constexpr uint32_t HEIGHT = 600;
};

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
    glfwDestroyWindow(m_window);

    glfwTerminate();
}