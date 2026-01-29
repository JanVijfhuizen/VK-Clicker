#include "pch.h"
#include "Buffer.h"
#include "VkCheck.h"

namespace gr {
	void Buffer::Destroy(const Core& core)
	{
		vkDestroyBuffer(core.device, value, nullptr);
		vkFreeMemory(core.device, memory, nullptr);
	}
	Buffer BufferBuilder::Build(const Core& core, 
		VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	{
		Buffer buffer{};

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkCheck(vkCreateBuffer(core.device, &bufferInfo, nullptr, &buffer.value));

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(core.device, buffer.value, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = core.FindMemoryType(
			memRequirements.memoryTypeBits,
			properties
		);

		VkCheck(vkAllocateMemory(core.device, &allocInfo, nullptr, &buffer.memory));
		vkBindBufferMemory(core.device, buffer.value, buffer.memory, 0);
		return buffer;
	}
}