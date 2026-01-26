#include "pch.h"
#include "Core.h"
#include "VkCheck.h"
#include "Vec.h"
#include "Set.h"

namespace gr
{
	Core CoreBuilder::Build(ARENA arena, Window& window)
	{
		assert(arena != TEMP);

		_core = {};
		_core.resolution = window.GetResolution();

		BuildInstance(arena, window);
		BuildSurface(window);
        BuildPhysicalDevice();
        BuildLogicalDevice();
        BuildDebugUtilsMessengerEXT();

		return _core;
	}
	CoreBuilder& CoreBuilder::AddGLFWSupport()
	{
		_glfwExtensions = glfwGetRequiredInstanceExtensions(&_glfwExtensionsCount);
		return *this;
	}
	CoreBuilder& CoreBuilder::SetValidationLayers(const mem::Arr<const char*>& layers)
	{
		_validationLayers = layers;
		return *this;
	}
	CoreBuilder& CoreBuilder::SetVkVersion(uint32_t version)
	{
		_version = version;
        return *this;
	}
    CoreBuilder& CoreBuilder::SetConcurrentPoolCount(uint32_t count)
    {
        _concurrentPoolCount = count;
        return *this;
    }
    CoreBuilder& CoreBuilder::EnableValidationLayers(bool on)
    {
        _enableValidationLayers = on;
        return *this;
    }
	void CoreBuilder::BuildInstance(ARENA arena, Window& window)
	{
		auto name = window.GetName();

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = name;
		appInfo.applicationVersion = _version;
		appInfo.pEngineName = name;
		appInfo.engineVersion = _version;
		appInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto _ = mem::scope(TEMP);

		auto layers = mem::Arr<const char*>(TEMP, _validationLayers.length() + 1);
		layers[0] = "VK_LAYER_KHRONOS_validation";
		layers.put(1, _validationLayers);
		createInfo.enabledLayerCount = _enableValidationLayers ? layers.length() : 0;
		createInfo.ppEnabledLayerNames = layers.ptr();

		auto extensions = mem::Arr<const char*>(TEMP, _glfwExtensionsCount + 1);
		extensions[0] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		extensions.put(1, _glfwExtensions, _glfwExtensionsCount);
		createInfo.enabledExtensionCount = extensions.length();
		createInfo.ppEnabledExtensionNames = extensions.ptr();

		VkCheck(vkCreateInstance(&createInfo, nullptr, &_core.instance));
	}
	void CoreBuilder::BuildSurface(Window& window)
	{
		VkCheck(glfwCreateWindowSurface(_core.instance, window.Ptr(), nullptr, &_core.surface));
	}
	void CoreBuilder::BuildPhysicalDevice()
	{
        struct Rateable {
            VkPhysicalDevice device;
            uint32_t rating = 0;
        };

        auto _ = mem::scope(TEMP);
        auto devices = GetPhysicalDevices();
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
        _core.physicalDevice = rateables[0].device;
	}

    void CoreBuilder::BuildLogicalDevice()
    {
        // Create queues.
        float queuePriority = 1;
        auto queueFamily = _core.queueFamily = GetQueueFamily();

        auto _ = mem::scope(TEMP);
        const uint32_t l = QUEUE_LEN;
        auto set = mem::Set<uint32_t>(TEMP, l);
        for (uint32_t i = 0; i < l; i++)
        {
            auto id = queueFamily.queues[i];
            set.insert(id, id);
        }

        auto familyVec = set.vec();
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

        VkCheck(vkCreateDevice(_core.physicalDevice, &createInfo, nullptr, &_core.device));

        auto closed = mem::Set<VkQueue>(TEMP, 4);

        for (uint32_t i = 0; i < 4; i++)
        {
            VkQueue queue;
            auto id = queueFamily.queues[i];
            if (!closed.contains(id, queue))
            {
                vkGetDeviceQueue(_core.device, id, 0, &queue);
                closed.insert(id, queue);
                _core.queues[i] = queue;
            }
            else
                _core.queues[i] = queue;
        }
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData)
    {
        std::cerr << "validation: " << callbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    void CoreBuilder::BuildDebugUtilsMessengerEXT()
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
        createInfo.pfnUserCallback = DebugCallback;
        createInfo.pUserData = nullptr; // optional
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(_core.instance, "vkCreateDebugUtilsMessengerEXT");
        auto result = func ? func(_core.instance, &createInfo, nullptr, &_core.debugMessenger)
            : VK_ERROR_EXTENSION_NOT_PRESENT;
        VkCheck(result);
    }

    mem::Arr<VkPhysicalDevice> CoreBuilder::GetPhysicalDevices()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(_core.instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("No Vulkan GPUs found!");
        }

        auto arr = mem::Arr<VkPhysicalDevice>(TEMP, deviceCount);
        vkEnumeratePhysicalDevices(_core.instance, &deviceCount, arr.ptr());
        return arr;
    }
    QueueFamily CoreBuilder::GetQueueFamily()
    {
        QueueFamily family{};
        for (uint32_t i = 0; i < QUEUE_LEN; i++)
            family.queues[i] = -1;

        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(_core.physicalDevice, &familyCount, nullptr);

        auto scope = mem::scope(TEMP);
        auto arr = mem::Arr<VkQueueFamilyProperties>(TEMP, familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(_core.physicalDevice, &familyCount, arr.ptr());

        uint32_t i = 0;
        arr.iterb([&family, &i, this](VkQueueFamilyProperties& current, auto) {
            if (current.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                family.queues[(int)QueueType::graphics] = i;
            }

            if (current.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                family.queues[(int)QueueType::transfer] = i;
            }

            if (current.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                family.queues[(int)QueueType::compute] = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(_core.physicalDevice, i, _core.surface, &presentSupport);
            if (presentSupport) {
                family.queues[(int)QueueType::present] = i;
            }

            if (family.Complete())
                return false;
            i++;
            return true;
            });

        return family;
    }
    void Core::OnScopeClear()
    {
        vkDeviceWaitIdle(device);

        auto DestroyDebugUtilsMessengerEXT =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (debugMessenger)
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    }
    uint32_t Core::FindMemoryType(uint32_t filter, VkMemoryPropertyFlags properties) const
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((filter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("No memory available of this type!");
    }
}