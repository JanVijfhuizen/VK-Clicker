#include "pch.h"
#include "SwapChain.h"
#include "SwapChainSupportDetails.h"
#include "VkCheck.h"
#include "Math.h"
#include "RenderPass.h"

namespace gr {
	void SwapChain::OnScopeClear()
	{
		Clear(true);
	}
	void SwapChain::Recreate(Window& window)
	{
		if (_swapChain)
			Clear(false);
		Create(-1, window);
	}
	void SwapChain::Frame(Window& window)
	{
		EndFrame();
		BeginFrame(window);
	}
	void SwapChain::BindResource(SwapChainResource* resource)
	{
		_resources.add(_arena) = resource;
		resource->OnCreate(*_core, *this);
	}
	void SwapChain::AllocCommandBuffers(QueueType type, uint32_t amount, VkCommandBuffer* cmdBuffers)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = _pools[_imageIndex][(int)type];
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = amount;
		vkAllocateCommandBuffers(_core->device, &allocInfo, cmdBuffers);
	}
	VkRenderPass SwapChain::GetRenderPass()
	{
		return _renderPass;
	}
	uint32_t SwapChain::GetFrameCount()
	{
		return _images.length();
	}
	VkCommandBuffer SwapChain::GetCmd()
	{
		return _cmd;
	}
	uint32_t SwapChain::GetIndex()
	{
		return _imageIndex;
	}
	void SwapChain::Create(ARENA arena, Window& window)
	{
		auto _ = mem::scope(TEMP);

		_resolution = window.GetResolution();

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
			queueFamily.queues[(int)QueueType::graphics],
			queueFamily.queues[(int)QueueType::present]
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
		// Destroy old swapchain if it exists.
		if(oldSwapChain)
			vkDestroySwapchainKHR(_core->device, oldSwapChain, nullptr);

		SetImages(arena, format, oldSwapChain);
		SetCommandPools(arena, oldSwapChain);

		auto renderPassBuilder = RenderPassBuilder();
		_renderPass = renderPassBuilder.Build(*_core, format.format);

		SetFrameBuffers(arena, oldSwapChain);
		SetFencesAndSemaphores();

		auto res = _resources.arr(TEMP);
		for (uint32_t i = 0; i < res.length(); i++)
			res[i]->OnCreate(*_core, *this);

		BeginFrame(window);
	}
	void SwapChain::Clear(bool destroySwapChain)
	{
		auto _ = mem::scope(TEMP);

		auto res = _resources.arr(TEMP);
		for (uint32_t i = 0; i < res.length(); i++)
			res[i]->OnDestroy(*_core, *this);

		vkDestroySemaphore(_core->device, _imageAvailableSemaphore, nullptr);
		vkDestroySemaphore(_core->device, _renderFinishedSemaphore, nullptr);
		vkDestroyFence(_core->device, _inFlightFence, nullptr);

		for (uint32_t i = 0; i < _frameBuffers.length(); i++)
			vkDestroyFramebuffer(_core->device, _frameBuffers[i], nullptr);

		vkDestroyRenderPass(_core->device, _renderPass, nullptr);

		for (uint32_t i = 0; i < _pools.length(); i++)
		{
			auto& pool = _pools[i];
			for (uint32_t j = 0; j < _images.length(); j++)
				vkDestroyCommandPool(_core->device, pool[j], nullptr);
		}

		for (uint32_t i = 0; i < _images.length(); i++)
			vkDestroyImageView(_core->device, _views[i], nullptr);

		if(destroySwapChain)
			vkDestroySwapchainKHR(_core->device, _swapChain, nullptr);
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
	void SwapChain::SetCommandPools(ARENA arena, VkSwapchainKHR oldSwapChain)
	{
		const uint32_t l = _images.length();
		if (!oldSwapChain)
			_pools = mem::Arr<mem::Arr<VkCommandPool>>(arena, l);

		// Create command pool
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		
		for (uint32_t i = 0; i < l; i++)
		{
			// Graphics, Compute, Transfer.
			if(!oldSwapChain)
				_pools[i] = mem::Arr<VkCommandPool>(arena, QUEUE_LEN - 1);
			auto& framePools = _pools[i];

			// Excluding present buffer.
			for (uint32_t j = 0; j < QUEUE_LEN - 1; j++)
			{
				poolInfo.queueFamilyIndex = _core->queueFamily.queues[j];
				VkCheck(vkCreateCommandPool(_core->device, &poolInfo, nullptr, &framePools[j]));
			}
		}
	}
	void SwapChain::SetFrameBuffers(ARENA arena, VkSwapchainKHR oldSwapChain)
	{
		if (!oldSwapChain)
			_frameBuffers = mem::Arr<VkFramebuffer>(arena, _images.length());

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = _renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.width = _extent.width;
		framebufferInfo.height = _extent.height;
		framebufferInfo.layers = 1;

		for (uint32_t i = 0; i < _images.length(); i++)
		{
			VkImageView attachments[] = { _views[i] };
			framebufferInfo.pAttachments = attachments;
			VkCheck(vkCreateFramebuffer(_core->device, &framebufferInfo, nullptr, &_frameBuffers[i]));
		}
	}
	void SwapChain::SetFencesAndSemaphores()
	{
		VkSemaphoreCreateInfo semInfo{};
		semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		vkCreateSemaphore(_core->device, &semInfo, nullptr, &_imageAvailableSemaphore);
		vkCreateSemaphore(_core->device, &semInfo, nullptr, &_renderFinishedSemaphore);
		vkCreateFence(_core->device, &fenceInfo, nullptr, &_inFlightFence);
	}
	void SwapChain::BeginFrame(Window& window)
	{
		auto device = _core->device;
		vkWaitForFences(device, 1, &_inFlightFence, VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &_inFlightFence);

		auto res = vkAcquireNextImageKHR(
			device,
			_swapChain,
			UINT64_MAX,
			_imageAvailableSemaphore,
			VK_NULL_HANDLE,
			&_imageIndex
		);

		if (res == VK_ERROR_OUT_OF_DATE_KHR)
		{
			Recreate(window);
			return;
		}

		// Rather than keeping tracks of command buffers, just reset the pool every time.
		auto& subPools = _pools[_imageIndex];
		for (uint32_t i = 0; i < (int)QUEUE_LEN - 1; i++)
			vkResetCommandPool(_core->device, subPools[i], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		
		AllocCommandBuffers(QueueType::graphics, 1, &_cmd);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		vkBeginCommandBuffer(_cmd, &beginInfo);

		VkClearValue clearColor{};
		clearColor.color = { {0.1f, 0.1f, 0.1f, 1.0f} };

		VkRenderPassBeginInfo rpInfo{};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpInfo.renderPass = _renderPass;
		rpInfo.framebuffer = _frameBuffers[_imageIndex];
		rpInfo.renderArea.offset = { 0, 0 };
		rpInfo.renderArea.extent = _extent;
		rpInfo.clearValueCount = 1;
		rpInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(_cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
	}
	void SwapChain::EndFrame()
	{
		vkCmdEndRenderPass(_cmd);
		vkEndCommandBuffer(_cmd);

		VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &_imageAvailableSemaphore;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &_cmd;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &_renderFinishedSemaphore;

		vkQueueSubmit(_core->queues[(int)QueueType::graphics], 1, &submitInfo, _inFlightFence);

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &_renderFinishedSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &_swapChain;
		presentInfo.pImageIndices = &_imageIndex;

		vkQueuePresentKHR(_core->queues[(int)QueueType::present], &presentInfo);
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
		return mode;
	}
	SwapChain SwapChainBuilder::Build(ARENA arena, Core& core, Window& window)
	{
		_swapChain._arena = arena;
		_swapChain._core = &core;
		_swapChain.Create(arena, window);
		return _swapChain;
	}
	SwapChainBuilder& SwapChainBuilder::SetPreferredPresentMode(PresentMode mode)
	{
		_swapChain._preferredPresentMode = mode;
		return *this;
	}
}