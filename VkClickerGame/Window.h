#pragma once

struct Window final : public mem::IScoped
{
	friend struct WindowBuilder;

	bool Update();
	virtual void OnScopeClear() override;

	GLFWwindow* Ptr();

private:
	GLFWwindow* _value;
};

struct WindowBuilder final
{
	Window Build();
	WindowBuilder& SetName(const char* name);
	WindowBuilder& SetResolution(glm::ivec2 resolution);

private:
	Window _instance{};
	const char* _name = "GLFW Window";
	glm::ivec2 _resolution{800, 600};
};

