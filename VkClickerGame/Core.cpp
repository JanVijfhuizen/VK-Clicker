#include "pch.h"
#include "Core.h"
#include "VkCheck.h"
#include "Vec.h"

namespace gr
{
	Core CoreBuilder::Build(ARENA arena, Window& window)
	{
		assert(arena != TEMP);

		_core = {};
		_core.resolution = window.GetResolution();
		_core.preferredPresentMode = _preferredPresentMode;

		BuildInstance(arena, window);
		BuildSurface(window);
        BuildPhysicalDevice();
        BuildLogicalDevice();

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
	CoreBuilder& CoreBuilder::SetPreferredPresentMode(PresentMode mode)
	{
		_preferredPresentMode = mode;
		return *this;
	}
	CoreBuilder& CoreBuilder::SetVkVersion(uint32_t version)
	{
		_version = version;
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
		createInfo.enabledLayerCount = layers.length();
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
}