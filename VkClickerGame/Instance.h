#pragma once
#include "Window.h"
#include "PresentMode.h"
#include "QueueFamily.h"

namespace gr
{
	struct Instance final : public mem::IScoped
	{
		friend struct InstanceBuilder;

		void Update();
		virtual void OnScopeClear() override;
		void RecreateSwapChain(Window& window);

	private:
		VkQueue _graphicsQueue;
		VkQueue _presentQueue;
		QueueFamily queueFamily;
		VkCommandPool _cmdGraphicsPool;
		VkCommandPool _cmdPresentPool;
		VkCommandPool _cmdTransferPool;
		VkCommandPool _cmdComputePool;

		// Default render pipeline.
		VkDescriptorSetLayout _descriptorSetLayout;
		VkDescriptorPool _descriptorPool;
		VkPipelineLayout _pipelineLayout;
		const char* _defaultVertPath;
		const char* _defaultFragPath;

		ARENA _arena;
		mem::Arr<VkImage> _images;
		mem::Arr<VkImageView> _views;
		mem::Arr<VkFramebuffer> _frameBuffers;
		mem::Arr<VkCommandBuffer> _cmdBuffers;
		mem::Arr<VkDescriptorSet> _descriptorSets;
		VkRenderPass _renderPass;
		VkExtent2D _extent;

		PresentMode _preferredPresentMode;
		glm::ivec2 _resolution;

		VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
		VkPipeline _pipeline;

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const mem::Arr<VkSurfaceFormatKHR>& formats);
		VkPresentModeKHR ChooseSwapPresentMode(const mem::Arr<VkPresentModeKHR>& modes);
		void SetSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		void CreateImages(VkSurfaceFormatKHR format, VkSwapchainKHR oldSwapChain);
		void CreateRenderPass(VkSurfaceFormatKHR format);
		void CreateFrameBuffers(VkSwapchainKHR oldSwapChain);
		void DestroySwapChain();
		void CreateDefaultPipeline();
		void CreateDescriptorPool();
	};

	struct InstanceBuilder final
	{
		Instance Build(ARENA arena, Window& window);
		InstanceBuilder& SetDefaultVertPath(const char* path);
		InstanceBuilder& SetDefaultFragPath(const char* path);

	private:
		Instance _instance{};
		const char* _name = "VK Instance";
		const char** _windowingExtensions;
		uint32_t _windowingExtensionsCount;
		mem::Arr<const char*> _validationLayers;
		PresentMode _preferredPresentMode = PresentMode::immediate;
		const char* _defaultVertPath = "vert.spv";
		const char* _defaultFragPath = "frag.spv";

		mem::Arr<VkPhysicalDevice> GetPhysicalDevices();
		void SetPhysicalDevice();
		void SetLogicalDevice();
		QueueFamily GetQueueFamily();
		VkResult CreateDebugUtilsMessengerEXT();
		void SetCommandPool();
		void CreateDefaultDescriptors();
		void CreateDefaultPipelineLayout();
	};
}