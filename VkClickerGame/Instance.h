#pragma once

struct Instance final
{
	friend struct InstanceBuilder;

private:
	VkInstance _value;
};

struct InstanceBuilder final
{
	Instance Build();

	InstanceBuilder& SetName(const char* name);
	InstanceBuilder& AddGLFWSupport();

private:
	Instance _instance{};
	const char* _name = "VK Instance";
	const char** _windowingExtensions;
	uint32_t _windowingExtensionsCount;
};

