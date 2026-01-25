#pragma once
#include <vulkan/vulkan_core.h>
#include "QueueFamily.h"
#include "Window.h"
#include "PresentMode.h"

namespace gr {
	struct Core final : public mem::IScoped {
		VkInstance instance;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkDebugUtilsMessengerEXT debugMessenger;
		QueueFamily queueFamily;
		glm::ivec2 resolution;
		PresentMode preferredPresentMode;
		VkQueue queues[(int)QueueFamily::Type::length];

		virtual void OnScopeClear() override;
	};

	struct CoreBuilder final {
		Core Build(ARENA arena, Window& window);

		CoreBuilder& AddGLFWSupport();
		CoreBuilder& SetValidationLayers(const mem::Arr<const char*>& layers);
		CoreBuilder& SetPreferredPresentMode(PresentMode mode);
		CoreBuilder& SetVkVersion(uint32_t version);
		CoreBuilder& SetConcurrentPoolCount(uint32_t count);

	private:
		Core _core{};
		uint32_t _version = VK_MAKE_VERSION(1, 0, 0);
		const char** _glfwExtensions = nullptr;
		uint32_t _glfwExtensionsCount;
		PresentMode _preferredPresentMode = PresentMode::immediate;
		mem::Arr<const char*> _validationLayers;
		uint32_t _concurrentPoolCount = 1;

		void BuildInstance(ARENA arena, Window& window);
		void BuildSurface(Window& window);
		void BuildPhysicalDevice();
		void BuildLogicalDevice();
		void BuildDebugUtilsMessengerEXT();

		mem::Arr<VkPhysicalDevice> GetPhysicalDevices();
		QueueFamily GetQueueFamily();
	};
}