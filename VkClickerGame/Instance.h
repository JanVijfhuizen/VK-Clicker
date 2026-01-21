#pragma once
#include "Window.h"

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

struct Instance final : public mem::IScoped
{
	friend struct InstanceBuilder;

	virtual void OnScopeClear() override;

private:
	VkInstance _value;
	VkSurfaceKHR _surface;
	VkPhysicalDevice _physicalDevice;
	VkDevice _device;
	VkQueue _graphicsQueue;
	VkDebugUtilsMessengerEXT _debugMessenger;
	QueueFamily queueFamily;
	VkCommandPool _cmdGraphicsPool;
	VkCommandPool _cmdPresentPool;
	VkCommandPool _cmdTransferPool;
	VkCommandPool _cmdComputePool;
};

struct InstanceBuilder final
{
	Instance Build(Window& window);

	InstanceBuilder& SetName(const char* name);
	InstanceBuilder& AddGLFWSupport();
	InstanceBuilder& SetValidationLayers(mem::Arr<const char*> layers);

private:
	Instance _instance{};
	const char* _name = "VK Instance";
	const char** _windowingExtensions;
	uint32_t _windowingExtensionsCount;
	mem::Arr<const char*> _validationLayers;

	mem::Arr<VkPhysicalDevice> GetPhysicalDevices(Instance& instance);
	void SetPhysicalDevice(Instance& instance);
	void SetLogicalDevice(Instance& instance);
	QueueFamily GetQueueFamily(Instance& instance);
	VkResult CreateDebugUtilsMessengerEXT(Instance& instance);
	void SetCommandPool(Instance& instance);
};

