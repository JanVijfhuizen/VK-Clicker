#pragma once
#include "Window.h"

struct Instance final : public mem::IScoped
{
	friend struct InstanceBuilder;

	virtual void OnScopeClear() override;

private:
	VkInstance _value;
	VkSurfaceKHR _surface;
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
};

