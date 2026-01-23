#pragma once
#include <vulkan/vulkan_core.h>
#include "QueueFamily.h"
#include "Window.h"
#include "PresentMode.h"

namespace gr {
	struct Core final {
		VkInstance instance;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkDebugUtilsMessengerEXT debugMessenger;
		QueueFamily queueFamily;
		glm::ivec2 resolution;
		PresentMode preferredPresentMode;
	};

	struct CoreBuilder final {
		Core Build(ARENA arena, Window& window);

		CoreBuilder& AddGLFWSupport();
		CoreBuilder& SetValidationLayers(const mem::Arr<const char*>& layers);
		CoreBuilder& SetPreferredPresentMode(PresentMode mode);
		CoreBuilder& SetVkVersion(uint32_t version);

	private:
		Core _core{};
		uint32_t _version = VK_MAKE_VERSION(1, 0, 0);
		const char** _glfwExtensions = nullptr;
		uint32_t _glfwExtensionsCount;
		PresentMode _preferredPresentMode = PresentMode::immediate;
		mem::Arr<const char*> _validationLayers;

		void BuildInstance(ARENA arena, Window& window);
		void BuildSurface(Window& window);
		void BuildPhysicalDevice();

		mem::Arr<VkPhysicalDevice> GetPhysicalDevices();
	};
}