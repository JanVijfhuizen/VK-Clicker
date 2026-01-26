#include "pch.h"
#include "DescriptorSetLayoutBuilder.h"

namespace gr {
	VkDescriptorSetLayout TEMP_DescriptorSetLayoutBuilder::Build(const Core& core)
	{
		auto _ = mem::scope(TEMP);
		auto bindings = _bindings.arr(TEMP);

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
}