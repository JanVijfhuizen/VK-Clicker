#include "pch.h"
#include "Instance.h"

int main()
{
    mem::init();

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(800, 600, "GLFW + Vulkan", nullptr, nullptr);

    auto instanceBuilder = InstanceBuilder(TEMP);
    instanceBuilder.SetName("My Engine").AddGLFWSupport().Build();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        mem::frame();
    }

    glfwTerminate();
    mem::end();
}