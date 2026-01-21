#include "pch.h"
#include "Instance.h"
#include <GLFW/glfw3.h>
#include "Vec.h"
#include "Math.h"

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

void InstanceBuilder::SetCommandPool(Instance& instance)
{
    // Create command pool
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = instance.queueFamily.graphics;

    if (vkCreateCommandPool(instance._device, &poolInfo, nullptr, &instance._cmdGraphicsPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics command pool!");
    }

    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = instance.queueFamily.present;
    if (vkCreateCommandPool(instance._device, &poolInfo, nullptr, &instance._cmdPresentPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create present command pool!");
    }

    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = instance.queueFamily.transfer;
    if (vkCreateCommandPool(instance._device, &poolInfo, nullptr, &instance._cmdTransferPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create transfer command pool!");
    }

    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = instance.queueFamily.compute;
    if (vkCreateCommandPool(instance._device, &poolInfo, nullptr, &instance._cmdComputePool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute command pool!");
    }
}

void Instance::RecreateSwapChain(Window& window)
{
    _resolution = window.GetResolution();

    VkSwapchainKHR oldSwapchain = _swapChain;
    if (oldSwapchain)
        DestroySwapChain();

    auto _ = mem::scope(TEMP);

    auto details = TEMP_GetSwapChainSupportDetails();
    auto format = ChooseSwapSurfaceFormat(details.formats);
    auto extent = ChooseSwapChainExtent(details.capabilities);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _surface;
    createInfo.minImageCount = details.capabilities.minImageCount + 1;
    if(details.capabilities.maxImageCount != 0)
        createInfo.minImageCount = jv::Min(createInfo.minImageCount, details.capabilities.maxImageCount);

    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    uint32_t queueFamilyIndices[] = {
        queueFamily.graphics, 
        queueFamily.present
    };

    // If sharing the same queue.
    if (queueFamily.graphics != queueFamily.present) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = details.capabilities.currentTransform;
    if (!(details.capabilities.supportedTransforms & details.capabilities.currentTransform))
        createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    auto flags = details.capabilities.supportedCompositeAlpha;
    if (!(flags & compositeAlpha)) {
        if (flags & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
            compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
        else if (flags & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
            compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
        else if (flags & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
            compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    }
    createInfo.compositeAlpha = compositeAlpha;
 
    createInfo.presentMode = ChooseSwapPresentMode(details.presentModes);
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapchain;

    auto result = vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain);
    if (result != VK_SUCCESS)
        throw std::runtime_error("Failed to create Swap Chain!");

    // Create images.
    uint32_t imageCount = _images.length();
    if (!oldSwapchain)
    {        
        vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);
        _images = mem::Arr<VkImage>(_arena, imageCount);
        _views = mem::Arr<VkImageView>(_arena, imageCount);
    }

    vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _images.ptr());

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    for (uint32_t i = 0; i < _images.length(); i++)
    {
        viewInfo.image = _images[i];
        vkCreateImageView(_device, &viewInfo, nullptr, &_views[i]);
    }
}

Instance::SwapChainSupportDetails Instance::TEMP_GetSwapChainSupportDetails()
{
    SwapChainSupportDetails details{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        _physicalDevice, _surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        _physicalDevice, _surface, &formatCount, nullptr);

    if (formatCount != 0) {
        auto& arr = details.formats = mem::Arr<VkSurfaceFormatKHR>(TEMP, formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            _physicalDevice, _surface, &formatCount, arr.ptr());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        _physicalDevice, _surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        auto& arr = details.presentModes = mem::Arr<VkPresentModeKHR>(TEMP, presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            _physicalDevice, _surface, &presentModeCount, arr.ptr());
    }

    return details;
}

VkSurfaceFormatKHR Instance::ChooseSwapSurfaceFormat(const mem::Arr<VkSurfaceFormatKHR>& formats)
{
    VkSurfaceFormatKHR format = formats[0];
    formats.iterb([&format](auto& f, auto) {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB &&
            f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            format = f;
            return false;
        }
        return true;
        });
    return format;
}

VkPresentModeKHR Instance::ChooseSwapPresentMode(const mem::Arr<VkPresentModeKHR>& modes)
{
    VkPresentModeKHR mode{};
    VkPresentModeKHR preferred;
    switch (_preferredPresentMode)
    {
    case PresentMode::immediate:
        preferred = VK_PRESENT_MODE_IMMEDIATE_KHR;
        break;
    case PresentMode::mailbox:
        preferred = VK_PRESENT_MODE_MAILBOX_KHR;
        break;
    case PresentMode::fifo:
    default:
        preferred = VK_PRESENT_MODE_FIFO_KHR;
        break;
    }

    modes.iterb([&mode, preferred](auto& m, auto) {
        if (m == preferred)
        {
            mode = m;
            return false;
        }
        return true;
        });
    return mode;
}

VkExtent2D Instance::ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;

    VkExtent2D actualExtent = { _resolution.x, _resolution.y };

    actualExtent.width = jv::Clamp(
        actualExtent.width,
        capabilities.minImageExtent.width,
        capabilities.maxImageExtent.width);

    actualExtent.height = jv::Clamp(
        actualExtent.height,
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height);

    return actualExtent;
}

void Instance::DestroySwapChain()
{
    for (uint32_t i = 0; i < _images.length(); i++)
        vkDestroyImageView(_device, _views[i], nullptr);
    vkDestroySwapchainKHR(_device, _swapChain, nullptr);
}

Instance InstanceBuilder::Build(ARENA arena, Window& window)
{
    assert(arena != TEMP);

    Instance instance{};
    instance._arena = arena;
    instance._resolution = window.GetResolution();
    instance._preferredPresentMode = _preferredPresentMode;

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

    if (vkCreateInstance(&createInfo, nullptr, &instance._value) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan instance!");

    if (glfwCreateWindowSurface(instance._value, window.Ptr(), nullptr, &instance._surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface!");

    SetPhysicalDevice(instance);
    SetLogicalDevice(instance);
    auto result = CreateDebugUtilsMessengerEXT(instance);
    if(result != VK_SUCCESS)
        throw std::runtime_error("Failed to create Debug Messenger!");
    
    SetCommandPool(instance);
    instance.RecreateSwapChain(window);
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

InstanceBuilder& InstanceBuilder::SetPreferredPresentMode(PresentMode mode)
{
    _preferredPresentMode = mode;
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
    auto queueFamily = instance.queueFamily = GetQueueFamily(instance);

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

    const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    if (vkCreateDevice(instance._physicalDevice, &createInfo, nullptr, &instance._device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    }

    vkGetDeviceQueue(instance._device, queueFamily.graphics, 0, &instance._graphicsQueue);

    if (queueFamily.graphics != queueFamily.present)
        vkGetDeviceQueue(instance._device, queueFamily.present, 0, &instance._presentQueue);
    else
        instance._presentQueue = instance._graphicsQueue;
}

QueueFamily InstanceBuilder::GetQueueFamily(Instance& instance)
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

        if (current.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            family.transfer = i;
        }

        if (current.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            family.compute = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(instance._physicalDevice, i, instance._surface, &presentSupport);
        if (presentSupport) {
            family.present = i;
        }

        if (family.Complete())
            return false;
        i++;
        return true;
        });
    
    return family;
}

void Instance::OnScopeClear()
{
    DestroySwapChain();

    vkDestroyCommandPool(_device, _cmdComputePool, nullptr);
    vkDestroyCommandPool(_device, _cmdTransferPool, nullptr);
    vkDestroyCommandPool(_device, _cmdPresentPool, nullptr);
    vkDestroyCommandPool(_device, _cmdGraphicsPool, nullptr);

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

bool QueueFamily::Complete()
{
    for (uint32_t i = 0; i < sizeof(queues) / sizeof(uint32_t); i++)
    {
        if (queues[i] == -1)
            return false;
    }
    return true;
}
