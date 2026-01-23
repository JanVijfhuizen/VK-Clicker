#pragma once

namespace gr
{
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		mem::Arr<VkSurfaceFormatKHR> formats{};
		mem::Arr<VkPresentModeKHR> presentModes{};
	};

	struct SwapChain final : public mem::IScoped
	{
		friend struct SwapChainBuilder;

		virtual void OnScopeClear() override;
		void Recreate();

	private:
		mem::Arr<VkImage> _images;
		mem::Arr<VkImageView> _views;
		mem::Arr<VkFramebuffer> _frameBuffers;
		mem::Arr<VkCommandBuffer> _cmdBuffers;

		VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
		VkSemaphore _imageAvailableSemaphore;
		VkSemaphore _renderFinishedSemaphore;
		VkFence _inFlightFence;

		void Create();
		void Clear();
	};

	struct SwapChainBuilder final
	{
		SwapChain Build();

	private:
		SwapChain _swapChain{};
	};

	SwapChainSupportDetails TEMP_GetSwapChainSupportDetails();
}