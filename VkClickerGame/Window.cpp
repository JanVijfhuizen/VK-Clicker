#include "pch.h"
#include "Window.h"

namespace gr {
	Window WindowBuilder::Build()
	{
		Window window{};
		window._resolution = _resolution;
		window._name = _name;
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window._value = glfwCreateWindow(_resolution.x, _resolution.y, _name, nullptr, nullptr);
		return window;
	}

	WindowBuilder& WindowBuilder::SetName(const char* name)
	{
		_name = name;
		return *this;
	}

	WindowBuilder& WindowBuilder::SetResolution(glm::ivec2 resolution)
	{
		_resolution = resolution;
		return *this;
	}

	bool Window::Update()
	{
		if (glfwWindowShouldClose(_value))
			return false;
		glfwPollEvents();
		return true;
	}

	void Window::OnScopeClear()
	{
		glfwDestroyWindow(_value);
		glfwTerminate();
	}

	glm::ivec2 Window::GetResolution()
	{
		return _resolution;
	}

	const char* Window::GetName()
	{
		return _name;
	}

	GLFWwindow* Window::Ptr()
	{
		return _value;
	}
}