#pragma once
#include "Core.h"

namespace gr
{
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		mem::Arr<VkSurfaceFormatKHR> formats{};
		mem::Arr<VkPresentModeKHR> presentModes{};
	};

	SwapChainSupportDetails TEMP_GetSwapChainSupportDetails(const Core& core);
}


