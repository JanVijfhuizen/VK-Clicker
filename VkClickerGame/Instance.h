#pragma once
#include "Window.h"

enum class PresentMode {
	immediate,
	mailbox,
	fifo
};

struct QueueFamily final {
	union
	{
		uint32_t queues[4]{UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
		struct
		{
			uint32_t graphics;
			uint32_t present;
			uint32_t transfer;
			uint32_t compute;
		};
	};
	
	bool Complete();
};

struct DefVertex final
{
	glm::vec2 pos{};
};

struct DefPushConstant final
{
	glm::mat4x4 transform;
};

struct Instance final : public mem::IScoped
{
	friend struct InstanceBuilder;

	virtual void OnScopeClear() override;
	void RecreateSwapChain(Window& window);

private:
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		mem::Arr<VkSurfaceFormatKHR> formats{};
		mem::Arr<VkPresentModeKHR> presentModes{};
	};

	VkInstance _value;
	VkSurfaceKHR _surface;
	VkPhysicalDevice _physicalDevice;
	VkDevice _device;
	VkQueue _graphicsQueue;
	VkQueue _presentQueue;
	VkDebugUtilsMessengerEXT _debugMessenger;
	QueueFamily queueFamily;
	VkCommandPool _cmdGraphicsPool;
	VkCommandPool _cmdPresentPool;
	VkCommandPool _cmdTransferPool;
	VkCommandPool _cmdComputePool;

	// Default render pipeline.
	VkDescriptorSetLayout _descriptorSetLayout;
	VkPipelineLayout _pipelineLayout;
	const char* _defaultVertPath;
	const char* _defaultFragPath;

	ARENA _arena;
	mem::Arr<VkImage> _images;
	mem::Arr<VkImageView> _views;
	mem::Arr<VkFramebuffer> _frameBuffers;
	VkRenderPass _renderPass;
	VkExtent2D _extent;

	PresentMode _preferredPresentMode;
	glm::ivec2 _resolution;

	VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
	VkPipeline _pipeline;

	SwapChainSupportDetails TEMP_GetSwapChainSupportDetails();
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const mem::Arr<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR ChooseSwapPresentMode(const mem::Arr<VkPresentModeKHR>& modes);
	void SetSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void CreateImages(VkSurfaceFormatKHR format, VkSwapchainKHR oldSwapChain);
	void CreateRenderPass(VkSurfaceFormatKHR format);
	void CreateFrameBuffers(VkSwapchainKHR oldSwapChain);
	void DestroySwapChain();
	void CreateDefaultPipeline();
};

struct InstanceBuilder final
{
	Instance Build(ARENA arena, Window& window);

	InstanceBuilder& SetName(const char* name);
	InstanceBuilder& AddGLFWSupport();
	InstanceBuilder& SetValidationLayers(mem::Arr<const char*> layers);
	InstanceBuilder& SetPreferredPresentMode(PresentMode mode);
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

