#pragma once
#include "Buffer.h"

namespace gr {
	struct TEMP_DescriptorWriter final
	{
		void Exec(const Core& core, VkDescriptorSet set);
		TEMP_DescriptorWriter& Add(uint32_t binding, const Buffer& buffer);
	private:
		mem::Link<VkDescriptorBufferInfo> _bufferInfos{};
		mem::Link<VkWriteDescriptorSet> _writes{};
	};
}