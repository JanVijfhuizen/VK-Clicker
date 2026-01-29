#pragma once
#include <vulkan/vulkan_core.h>
#include "Queues.h"
#include "Window.h"
#include "PresentMode.h"

namespace gr {
	struct Core final : public mem::IScoped {
		VkInstance instance;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkDebugUtilsMessengerEXT debugMessenger;
		Queues queueFamily;
		glm::ivec2 resolution;
		VkQueue queues[Queues::length];

		virtual void OnScopeClear() override;

		uint32_t FindMemoryType(uint32_t filter, VkMemoryPropertyFlags properties) const;
	};

	struct CoreBuilder final {
		Core Build(ARENA arena, Window& window);

		CoreBuilder& AddGLFWSupport();
		CoreBuilder& SetValidationLayers(const mem::Arr<const char*>& layers);
		CoreBuilder& SetVkVersion(uint32_t version);
		CoreBuilder& SetConcurrentPoolCount(uint32_t count);
		CoreBuilder& EnableValidationLayers(bool on);

	private:
		Core _core{};
		uint32_t _version = VK_MAKE_VERSION(1, 0, 0);
		const char** _glfwExtensions = nullptr;
		uint32_t _glfwExtensionsCount;
		PresentMode _preferredPresentMode = PresentMode::immediate;
		mem::Arr<const char*> _validationLayers;
		uint32_t _concurrentPoolCount = 1;
		bool _enableValidationLayers = true;

		void BuildInstance(ARENA arena, Window& window);
		void BuildSurface(Window& window);
		void BuildPhysicalDevice();
		void BuildLogicalDevice();
		void BuildDebugUtilsMessengerEXT();

		mem::Arr<VkPhysicalDevice> GetPhysicalDevices();
		Queues GetQueueFamily();
	};
}