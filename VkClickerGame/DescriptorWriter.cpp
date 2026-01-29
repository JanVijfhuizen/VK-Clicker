#include "pch.h"
#include "DescriptorWriter.h"
#include "Vec.h"

namespace gr {
	void TEMP_DescriptorWriter::Exec(const Core& core, VkDescriptorSet set)
	{
		auto _ = mem::scope(TEMP);

		auto writes = _writes.arr(TEMP);
		for (uint32_t i = 0; i < writes.length(); i++)
			writes[i].dstSet = set;

		vkUpdateDescriptorSets(
			core.device,
			writes.length(),
			writes.ptr(),
			0,
			nullptr
		);
	}
	TEMP_DescriptorWriter& TEMP_DescriptorWriter::Add(uint32_t binding, const Buffer& buffer)
	{
		auto& info = _bufferInfos.add(TEMP);
		info.buffer = buffer.value;
		info.offset = 0;
		info.range = buffer.size;

		auto& write = _writes.add(TEMP);
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = binding;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.descriptorCount = 1;
		write.pBufferInfo = &info;

		return *this;
	}
}