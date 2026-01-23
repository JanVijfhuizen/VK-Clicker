#pragma once

namespace gr
{
	struct Window final : public mem::IScoped
	{
		friend struct WindowBuilder;

		bool Update();
		virtual void OnScopeClear() override;
		glm::ivec2 GetResolution();
		const char* GetName();

		GLFWwindow* Ptr();

	private:
		GLFWwindow* _value;
		glm::ivec2 _resolution;
		const char* _name;
	};

	struct WindowBuilder final
	{
		Window Build();
		WindowBuilder& SetName(const char* name);
		WindowBuilder& SetResolution(glm::ivec2 resolution);

	private:
		Window _instance{};
		const char* _name = "GLFW Window";
		glm::ivec2 _resolution{ 800, 600 };
	};
}

