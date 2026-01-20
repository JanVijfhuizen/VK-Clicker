#include "pch.h"
#include "Instance.h"
#include <GLFW/glfw3.h>
#include "Vec.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData)
{
    std::cerr << "validation: " << callbackData->pMessage << std::endl;
    return VK_FALSE;
}

VkResult InstanceBuilder::CreateDebugUtilsMessengerEXT(Instance& instance)
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // optional
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance._value, "vkCreateDebugUtilsMessengerEXT");
    return func ? func(instance._value, &createInfo, nullptr, &instance._debugMessenger)
        : VK_ERROR_EXTENSION_NOT_PRESENT;
}

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

    auto _ = mem::scope(TEMP);

    auto layers = mem::Arr<const char*>(TEMP, _validationLayers.length() + 1);
    layers[0] = "VK_LAYER_KHRONOS_validation";
    layers.put(1, _validationLayers);
    createInfo.enabledLayerCount = layers.length();
    createInfo.ppEnabledLayerNames = layers.ptr();

    auto extensions = mem::Arr<const char*>(TEMP, _windowingExtensionsCount + 1);
    extensions[0] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    extensions.put(1, _windowingExtensions, _windowingExtensionsCount);
    createInfo.enabledExtensionCount = extensions.length();
    createInfo.ppEnabledExtensionNames = extensions.ptr();

    if (vkCreateInstance(&createInfo, nullptr, &instance._value) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance!");
    }

    if (glfwCreateWindowSurface(instance._value, window.Ptr(), nullptr, &instance._surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }

    SetPhysicalDevice(instance);
    SetLogicalDevice(instance);
    CreateDebugUtilsMessengerEXT(instance);

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

void InstanceBuilder::SetPhysicalDevice(Instance& instance)
{
    struct Rateable {
        VkPhysicalDevice device;
        uint32_t rating = 0;
    };

    auto _ = mem::scope(TEMP);
    auto devices = GetPhysicalDevices(instance);
    auto requiredExtensions = mem::Arr<const char*>(TEMP, 1);
    requiredExtensions[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

    auto rateables = mem::Vec<Rateable>(TEMP, devices.length());
    devices.iter([&rateables](auto& device, auto i) {
        auto& r = rateables.add() = {};
        r.device = device;
        });

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    auto availableExtensions = mem::Arr<VkExtensionProperties>(TEMP, extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.ptr());

    for (int32_t i = rateables.count() - 1; i >= 0; i--)
    {
        auto& rateable = rateables[i];

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(rateable.device, &features);

        if (!features.samplerAnisotropy)
            continue;

        // Check if this CPU can support ALL required extensions.
        bool valid = true;
        for (uint32_t j = 0; j < requiredExtensions.length(); j++)
        {
            auto required = requiredExtensions[j];
            bool found = false;

            // Can use a hashset for this but this is a small project anyway.
            for (uint32_t j = 0; j < availableExtensions.length(); j++)
            {
                auto available = availableExtensions[j];
                if (strcmp(required, available.extensionName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                valid = false;
                break;
            }
        }

        VkPhysicalDeviceProperties properties;
        
        vkGetPhysicalDeviceProperties(rateable.device, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            rateable.rating += 1e4;

        // Larger textures are better
        rateable.rating += properties.limits.maxImageDimension2D;

        // VRAM size
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(rateable.device, &memProps);

        VkDeviceSize localHeapSize = 0;
        for (uint32_t i = 0; i < memProps.memoryHeapCount; i++)
        {
            if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                localHeapSize += memProps.memoryHeaps[i].size;
        }

        rateable.rating += static_cast<uint32_t>(localHeapSize / (1024 * 1024 * 1024)); // GB
    }

    // Sort by rating.
    rateables.sort([](Rateable& a, Rateable& b) {
        return a.rating > b.rating;
    });
    instance._physicalDevice = rateables[0].device;
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
    auto DestroyDebugUtilsMessengerEXT =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_value, "vkDestroyDebugUtilsMessengerEXT");

    if (_debugMessenger != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(_value, _debugMessenger, nullptr);
    }

    vkDeviceWaitIdle(_device);
    vkDestroyDevice(_device, nullptr);
    vkDestroySurfaceKHR(_value, _surface, nullptr);
    vkDestroyInstance(_value, nullptr);
}

bool InstanceBuilder::QueueFamily::Valid()
{
    return graphics != -1 && present != -1;
}
