#pragma once
#include "Window.h"

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
};

struct InstanceBuilder final
{
	Instance Build(Window& window);

	InstanceBuilder& SetName(const char* name);
	InstanceBuilder& AddGLFWSupport();
	InstanceBuilder& SetValidationLayers(mem::Arr<const char*> layers);

private:
	struct QueueFamily final {
		uint32_t graphics = -1;
		uint32_t present = -1;

		bool Valid();
	};

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
};

