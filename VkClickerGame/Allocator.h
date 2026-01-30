#pragma once
#include "Core.h"
#include "Memory.h"

namespace gr {
	struct Allocator final
	{
		friend struct AllocatorBuilder;

		Memory Alloc(VkDeviceSize size, VkDeviceSize alignment);
		Memory Alloc(VkBuffer buffer, bool bind);
		void Clear();
		void Destroy();

	private:
		Core* _core;
		VkDeviceMemory _memory;
		uint32_t _memoryTypeIndex;

		VkDeviceSize _capacity;
		VkDeviceSize _offset;

		VkDeviceSize AlignUp(VkDeviceSize v, VkDeviceSize alignment);
	};

	struct AllocatorBuilder final {
		Allocator Build(const Core& core, uint32_t memoryTypeIndex, VkDeviceSize size);
	private:
		Allocator _allocator{};
	};
}
