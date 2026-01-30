#include "pch.h"
#include "Allocator.h"

namespace gr {
	Allocator AllocatorBuilder::Build(const Core& core, uint32_t memoryTypeIndex, VkDeviceSize size)
	{
		_allocator._core = &core;
		_allocator._capacity = size;
		_allocator._memoryTypeIndex = memoryTypeIndex;

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = size;
		allocInfo.memoryTypeIndex = memoryTypeIndex;

		vkAllocateMemory(core.device, &allocInfo, nullptr, &_allocator._memory);
		return _allocator;
	}
	Memory Allocator::Alloc(VkDeviceSize size, VkDeviceSize alignment)
	{
		VkDeviceSize alignedOffset =
			AlignUp(_offset, alignment);

		assert(alignedOffset + size <= _capacity);

		Memory mem{};
		mem.value = _memory;
		mem.offset = alignedOffset;
		mem.size = size;

		_offset = alignedOffset + size;
		return mem;
	}
	Memory Allocator::Alloc(VkBuffer buffer, bool bind)
	{
		VkMemoryRequirements req;
		vkGetBufferMemoryRequirements(_core->device, buffer, &req);
		auto mem = Alloc(req.size, req.alignment);
		if (bind)
			vkBindBufferMemory(_core->device, buffer, mem.value, mem.offset);
		return mem;
	}
	void Allocator::Clear()
	{
		_offset = 0;
	}
	void Allocator::Destroy()
	{
		vkFreeMemory(_core->device, _memory, nullptr);
	}
	VkDeviceSize Allocator::AlignUp(VkDeviceSize v, VkDeviceSize alignment)
	{
		return (v + alignment - 1) & ~(alignment - 1);
	}
}