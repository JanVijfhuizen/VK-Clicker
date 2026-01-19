#include "pch.h"
#include "Instance.h"
#include <GLFW/glfw3.h>
#include "Vec.h"

Instance InstanceBuilder::Build(Window& window)
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

    if (glfwCreateWindowSurface(instance._value, window.Ptr(), nullptr, &instance._surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }

    auto devices = GetPhysicalDevices(instance);
    // TEMP.
    instance._physicalDevice = devices[0];

    SetLogicalDevice(instance);

    return instance;
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

mem::Arr<VkPhysicalDevice> InstanceBuilder::GetPhysicalDevices(Instance& instance)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance._value, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("No Vulkan GPUs found!");
    }

    auto arr = mem::Arr<VkPhysicalDevice>(TEMP, deviceCount);
    vkEnumeratePhysicalDevices(instance._value, &deviceCount, arr.ptr());
    return arr;
}

void InstanceBuilder::SetLogicalDevice(Instance& instance)
{
    // Create queues.
    float queuePriority = 1;
    auto queueFamily = GetQueueFamily(instance);

    // Very hacky but it's whatever.
    auto familyVec = mem::Vec<uint32_t>(TEMP, 2);
    familyVec.add() = queueFamily.graphics;
    if(familyVec[0] != queueFamily.present)
        familyVec.add() = queueFamily.present;

    auto queueCreateInfos = mem::Arr<VkDeviceQueueCreateInfo>(TEMP, familyVec.count());
    familyVec.arr().iter([&queueCreateInfos, &queueFamily, &queuePriority](auto& family, auto i) {
        auto& info = queueCreateInfos[i] = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = family;
        info.queueCount = 1;
        info.pQueuePriorities = &queuePriority;
        });

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = queueCreateInfos.length();
    createInfo.pQueueCreateInfos = queueCreateInfos.ptr();

    if (vkCreateDevice(instance._physicalDevice, &createInfo, nullptr, &instance._device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(instance._device, 0, 0, &instance._graphicsQueue);
}

InstanceBuilder::QueueFamily InstanceBuilder::GetQueueFamily(Instance& instance)
{
    QueueFamily family;

    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(instance._physicalDevice, &familyCount, nullptr);

    auto scope = mem::scope(TEMP);
    auto arr = mem::Arr<VkQueueFamilyProperties>(TEMP, familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(instance._physicalDevice, &familyCount, arr.ptr());

    uint32_t i = 0;
    arr.iterb([&family, &i, &instance](VkQueueFamilyProperties& current, auto) {
        if (current.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            family.graphics = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(instance._physicalDevice, i, instance._surface, &presentSupport);
        if (presentSupport) {
            family.present = i;
        }

        if (family.Valid())
            return false;
        i++;
        return true;
        });
    
    return family;
}

void Instance::OnScopeClear()
{
    vkDeviceWaitIdle(_device);
    vkDestroyDevice(_device, nullptr);
    vkDestroySurfaceKHR(_value, _surface, nullptr);
    vkDestroyInstance(_value, nullptr);
}

bool InstanceBuilder::QueueFamily::Valid()
{
    return graphics != -1 && present != -1;
}
