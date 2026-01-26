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
		void Recreate(Window& window);

	private:
		Core* _core;
		PresentMode _preferredPresentMode;
		glm::ivec2 _resolution;
		VkExtent2D _extent;

		mem::Arr<mem::Arr<VkCommandPool>> _pools;
		mem::Arr<VkImage> _images;
		mem::Arr<VkImageView> _views;
		mem::Arr<VkFramebuffer> _frameBuffers;

		VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
		VkSemaphore _imageAvailableSemaphore;
		VkSemaphore _renderFinishedSemaphore;
		VkFence _inFlightFence;

		void Create(ARENA arena, Window& window);
		void Clear();

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const mem::Arr<VkSurfaceFormatKHR>& formats);
		VkPresentModeKHR ChooseSwapPresentMode(const mem::Arr<VkPresentModeKHR>& modes);
		void SetSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void SetImages(ARENA arena, VkSurfaceFormatKHR format, VkSwapchainKHR oldSwapChain);
		void SetCommandPools(ARENA arena);
	};

	struct SwapChainBuilder final
	{
		SwapChain Build(ARENA arena, Core& core, Window& window);
		SwapChainBuilder& SetPreferredPresentMode(PresentMode mode);

	private:
		SwapChain _swapChain{};
	};
}