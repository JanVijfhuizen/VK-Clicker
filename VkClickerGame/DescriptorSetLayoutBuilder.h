#pragma once
#include "Core.h"

namespace gr {
	struct DescriptorSetLayoutManager;

	enum class BindingType {
		ubo
	};

	enum class BindingStep {
		fragment,
		vertex
	};

	struct Binding final {
		uint32_t count = 1;
		BindingType type = BindingType::ubo;
		BindingStep step = BindingStep::vertex;

		uint64_t Hash();
	};

	struct TEMP_DescriptorSetLayoutBuilder final
	{
		VkDescriptorSetLayout Build(const Core& core, DescriptorSetLayoutManager& manager);
		TEMP_DescriptorSetLayoutBuilder& AddBinding(BindingType type, BindingStep step, uint32_t count = 1);
	private:
		mem::Link<Binding> _bindings{};
	};
}