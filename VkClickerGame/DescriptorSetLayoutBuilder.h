#pragma once
#include "Core.h"

namespace gr {
	enum class BindingType {
		ubo
	};

	enum class BindingStep {
		fragment,
		vertex
	};

	struct TEMP_DescriptorSetLayoutBuilder final
	{
		VkDescriptorSetLayout Build(Core& core);
		TEMP_DescriptorSetLayoutBuilder& AddBinding(BindingType type, BindingStep step, uint32_t count = 1);
	private:
		struct Binding final {
			uint32_t count = 1;
			BindingType type = BindingType::ubo;
			BindingStep step = BindingStep::vertex;
		};

		mem::Link<Binding> _bindings;
	};
}