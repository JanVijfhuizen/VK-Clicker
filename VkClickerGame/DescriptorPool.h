#pragma once
#include "Core.h"
#include "Bindingtype.h"

namespace gr {
	struct DescriptorPool final
	{
		friend struct DescriptorPoolBuilder;

		mem::Arr<VkDescriptorSet> Alloc(ARENA arena, const Core& core, VkDescriptorSetLayout layout, uint32_t amount);
		void Reset(const Core& core);
		void Destroy(const Core& core);

	private:
		VkDescriptorPool _value;
		uint32_t _maxSets;
	};

	struct DescriptorPoolBuilder final {
		DescriptorPool Build(const Core& core, BindingType* types, uint32_t* sizes, uint32_t length, uint32_t maxSets);
	};
}