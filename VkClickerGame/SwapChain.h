#pragma once
#include "Core.h"

namespace gr
{
	struct SwapChainResource {
		friend class SwapChain;
	protected:
		virtual void OnCreate(const Core& core, SwapChain& swapChain) = 0;
		virtual void OnDestroy(const Core& core, SwapChain& swapChain) = 0;
	};

	struct SwapChain final : public mem::IScoped
	{
		friend struct SwapChainBuilder;

		virtual void OnScopeClear() override;
		void Recreate(Window& window);
		void Frame(Window& window);

		void BindResource(SwapChainResource* resource);
		void AllocCommandBuffers(QueueType type, uint32_t amount, VkCommandBuffer* cmdBuffers);
		VkRenderPass GetRenderPass();

	private:
		ARENA _arena;
		Core* _core;
		mem::Link<SwapChainResource*> _resources{};
		PresentMode _preferredPresentMode = PresentMode::mailbox;
		glm::ivec2 _resolution;
		VkExtent2D _extent;

		mem::Arr<mem::Arr<VkCommandPool>> _pools;
		mem::Arr<VkImage> _images;
		mem::Arr<VkImageView> _views;
		mem::Arr<VkFramebuffer> _frameBuffers;

		VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
		VkRenderPass _renderPass;

		VkSemaphore _imageAvailableSemaphore;
		VkSemaphore _renderFinishedSemaphore;
		VkFence _inFlightFence;

		uint32_t _imageIndex;
		VkCommandBuffer _cmd;

		void Create(ARENA arena, Window& window);
		void Clear(bool destroySwapChain);

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const mem::Arr<VkSurfaceFormatKHR>& formats);
		VkPresentModeKHR ChooseSwapPresentMode(const mem::Arr<VkPresentModeKHR>& modes);
		void SetSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void SetImages(ARENA arena, VkSurfaceFormatKHR format, VkSwapchainKHR oldSwapChain);
		void SetCommandPools(ARENA arena, VkSwapchainKHR oldSwapChain);
		void SetFrameBuffers(ARENA arena, VkSwapchainKHR oldSwapChain);
		void SetFencesAndSemaphores();

		void BeginFrame(Window& window);
		void EndFrame();
	};

	struct SwapChainBuilder final
	{
		SwapChain Build(ARENA arena, Core& core, Window& window);
		SwapChainBuilder& SetPreferredPresentMode(PresentMode mode);

	private:
		SwapChain _swapChain{};
	};
}