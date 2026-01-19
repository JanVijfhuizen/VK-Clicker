#include "pch.h"
#include "Instance.h"
#include <GLFW/glfw3.h>

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

    auto _ = mem::scope(TEMP);
    auto arr = mem::Arr<const char*>(TEMP, _validationLayers.length() + 1);
    arr[0] = "VK_LAYER_KHRONOS_validation";
    arr.put(1, _validationLayers);

    createInfo.enabledLayerCount = arr.length();
    createInfo.ppEnabledLayerNames = arr.ptr();

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

InstanceBuilder& InstanceBuilder::SetValidationLayers(mem::Arr<const char*> layers)
{
    _validationLayers = layers;
    return *this;
}
