#include "pch.h"
#include "SwapChain.h"
#include "SwapChainSupportDetails.h"
#include "VkCheck.h"
#include "Math.h"

namespace gr {
	void SwapChain::OnScopeClear()
	{
		Clear();
	}
	void SwapChain::Recreate(Window& window)
	{
		_resolution = window.GetResolution();
		if (_swapChain)
			Clear();
		Create(-1, window);
	}
	void SwapChain::Create(ARENA arena, Window& window)
	{
		auto _ = mem::scope(TEMP);

		auto details = TEMP_GetSwapChainSupportDetails(*_core);
		auto format = ChooseSwapSurfaceFormat(details.formats);
		SetSwapChainExtent(details.capabilities);

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = _core->surface;
		createInfo.minImageCount = details.capabilities.minImageCount + 1;
		if (details.capabilities.maxImageCount != 0)
			createInfo.minImageCount = jv::Min(createInfo.minImageCount, details.capabilities.maxImageCount);

		createInfo.imageFormat = format.format;
		createInfo.imageColorSpace = format.colorSpace;
		createInfo.imageExtent = _extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		auto queueFamily = _core->queueFamily;

		uint32_t queueFamilyIndices[] = {
			queueFamily.queues[(int)QueueFamily::Type::graphics],
			queueFamily.queues[(int)QueueFamily::Type::present]
		};

		// If not sharing the same queue.
		if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
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

		auto oldSwapChain = _swapChain;

		createInfo.presentMode = ChooseSwapPresentMode(details.presentModes);
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = oldSwapChain;

		VkCheck(vkCreateSwapchainKHR(_core->device, &createInfo, nullptr, &_swapChain));
		SetImages(arena, format, oldSwapChain);
		SetCommandPools(arena);
	}
	void SwapChain::Clear()
	{
		vkDeviceWaitIdle(_core->device);
	}
	VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const mem::Arr<VkSurfaceFormatKHR>& formats)
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
	void SwapChain::SetSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
			_extent = capabilities.currentExtent;

		VkExtent2D actualExtent = { _resolution.x, _resolution.y };

		actualExtent.width = jv::Clamp(
			actualExtent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width);

		actualExtent.height = jv::Clamp(
			actualExtent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height);
		_extent = actualExtent;
	}
	void SwapChain::SetImages(ARENA arena, VkSurfaceFormatKHR format, VkSwapchainKHR oldSwapChain)
	{
		// Create images.
		uint32_t imageCount = _images.length();
		if (!oldSwapChain)
		{
			vkGetSwapchainImagesKHR(_core->device, _swapChain, &imageCount, nullptr);
			_images = mem::Arr<VkImage>(arena, imageCount);
			_views = mem::Arr<VkImageView>(arena, imageCount);
		}

		vkGetSwapchainImagesKHR(_core->device, _swapChain, &imageCount, _images.ptr());

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
			vkCreateImageView(_core->device, &viewInfo, nullptr, &_views[i]);
		}
	}
	void SwapChain::SetCommandPools(ARENA arena)
	{
		// Only create this once.
		const uint32_t l = _images.length();
		if (l != 0)
			return;

		// Create command pool
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		
		_pools = mem::Arr<mem::Arr<VkCommandPool>>(arena, l);
		for (uint32_t i = 0; i < l; i++)
		{
			// Graphics, Compute, Transfer.
			auto& framePools = _pools[i] = mem::Arr<VkCommandPool>(arena, QUEUE_LEN - 1);

			// Excluding present buffer.
			for (uint32_t j = 0; j < QUEUE_LEN - 1; j++)
			{
				poolInfo.queueFamilyIndex = _core->queueFamily.queues[j];
				VkCheck(vkCreateCommandPool(_core->device, &poolInfo, nullptr, &framePools[j]));
			}
		}
	}
	VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const mem::Arr<VkPresentModeKHR>& modes)
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
	}
	SwapChain SwapChainBuilder::Build(ARENA arena, Core& core, Window& window)
	{
		
	}
	SwapChainBuilder& SwapChainBuilder::SetPreferredPresentMode(PresentMode mode)
	{
		_swapChain._preferredPresentMode = mode;
		return *this;
	}
}