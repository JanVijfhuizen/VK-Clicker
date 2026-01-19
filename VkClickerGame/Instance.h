#pragma once

struct Instance final
{
	friend struct InstanceBuilder;

private:
	VkInstance _value;
};

struct InstanceBuilder final
{
	InstanceBuilder(ARENA arena);
	Instance Build();

	InstanceBuilder& SetName(const char* name);
	InstanceBuilder& AddGLFWSupport();

private:
	Instance _instance{};
	const char* _name;
	const char** _windowingExtensions;
	uint32_t _windowingExtensionsCount;
};

