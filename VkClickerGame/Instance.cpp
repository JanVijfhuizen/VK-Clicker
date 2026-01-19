#include "pch.h"
#include "Instance.h"
#include <GLFW/glfw3.h>

InstanceBuilder::InstanceBuilder(ARENA arena)
{
	_name = "VK Instance";
}

Instance InstanceBuilder::Build()
{
    Instance instance{};

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = _name;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = _name;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = _windowingExtensionsCount;
    createInfo.ppEnabledExtensionNames = _windowingExtensions;

    // No validation layers yet
    createInfo.enabledLayerCount = 0;

    if (vkCreateInstance(&createInfo, nullptr, &instance._value) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance!");
    }
}

InstanceBuilder& InstanceBuilder::SetName(const char* name)
{
    _name = name;
    return *this;
}

InstanceBuilder& InstanceBuilder::AddGLFWSupport()
{
    _windowingExtensions = glfwGetRequiredInstanceExtensions(&_windowingExtensionsCount);
    return *this;
}
