#pragma once

struct Window final
{
	friend struct WindowBuilder;

	bool Update();
	void Destroy();

private:
	GLFWwindow* _value;
};

struct WindowBuilder final
{
	Window Build();
	WindowBuilder& SetName(const char* name);

private:
	Window _instance{};
	const char* _name = "GLFW Window";
	glm::vec2 _resolution{800, 600};
};

