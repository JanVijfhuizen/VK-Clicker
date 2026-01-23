#include "pch.h"
#include "SwapChainSupportDetails.h"

namespace gr
{
	SwapChainSupportDetails TEMP_GetSwapChainSupportDetails(const Core& core)
	{
        SwapChainSupportDetails details{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            core.physicalDevice, core.surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            core.physicalDevice, core.surface, &formatCount, nullptr);

        if (formatCount != 0) {
            auto& arr = details.formats = mem::Arr<VkSurfaceFormatKHR>(TEMP, formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                core.physicalDevice, core.surface, &formatCount, arr.ptr());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            core.physicalDevice, core.surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            auto& arr = details.presentModes = mem::Arr<VkPresentModeKHR>(TEMP, presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                core.physicalDevice, core.surface, &presentModeCount, arr.ptr());
        }

        return details;
	}
}
