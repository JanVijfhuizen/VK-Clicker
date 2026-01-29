#include "pch.h"
#include "DescriptorSetLayoutBuilder.h"
#include "DescriptorSetLayoutManager.h"

namespace gr {
	VkDescriptorSetLayout TEMP_DescriptorSetLayoutBuilder::Build(const Core& core, DescriptorSetLayoutManager& manager)
	{
		auto _ = mem::scope(TEMP);
		auto bindings = _bindings.arr(TEMP);

		auto keys = mem::Arr<uint64_t>(TEMP, bindings.length());
		for (uint32_t i = 0; i < bindings.length(); i++)
			keys[i] = bindings[i].Hash();

		// Look if this set already exists.
		auto existingSets = manager._sets.arr(TEMP);
		for (uint32_t i = 0; i < existingSets.length(); i++)
		{
			auto& existingSet = existingSets[i];
			auto& oKeys = existingSet.keys;
			if (oKeys.length() != bindings.length())
				continue;

			bool valid = true;
			for (uint32_t j = 0; j < oKeys.length(); j++)
			{
				if (oKeys[i] == keys[i])
					continue;
				valid = false;
				break;
			}

			if (valid)
				return existingSet.layout;
		}

		auto ptrs = mem::Arr<VkDescriptorSetLayoutBinding>(TEMP, bindings.length());
		for (uint32_t i = 0; i < bindings.length(); i++)
		{
			auto& binding = bindings[i];
			auto& ptr = ptrs[i] = {};

			ptr.binding = i;
			ptr.descriptorCount = binding.count;
			ptr.pImmutableSamplers = nullptr;

			switch (binding.step)
			{
			case BindingStep::vertex:
				ptr.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				break;
			case BindingStep::fragment:
				ptr.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				break;
			default:
				break;
			}

			switch (binding.type)
			{
			case BindingType::ubo:
				ptr.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			default:
				break;
			}
		}

		VkDescriptorSetLayoutCreateInfo setLayoutInfo{};
		setLayoutInfo.sType =
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setLayoutInfo.bindingCount = ptrs.length();
		setLayoutInfo.pBindings = ptrs.ptr();
		setLayoutInfo.flags = 0;

		VkDescriptorSetLayout layout;
		vkCreateDescriptorSetLayout(core.device, &setLayoutInfo, nullptr, &layout);

		auto set = DescriptorSetLayoutManager::Set();
		set.keys = keys.copy(PERS);
		set.layout = layout;
		manager._sets.add(PERS) = set;

		return layout;
	}
	TEMP_DescriptorSetLayoutBuilder& TEMP_DescriptorSetLayoutBuilder::AddBinding(
		BindingType type, BindingStep step, uint32_t count)
	{
		Binding binding{};
		binding.type = type;
		binding.step = step;
		binding.count = count;
		_bindings.add(TEMP) = binding;
		return *this;
	}
	uint64_t Binding::Hash()
	{
		return count | ((uint64_t)type << 4) | ((uint64_t)step << 8);
	}
}