#pragma once
#include "Core.h"

namespace gr {
	struct Buffer final {
		VkBuffer value;
		VkDeviceMemory memory;
		void Destroy(const Core& core);
	};

	struct BufferBuilder final
	{
		Buffer Build(const Core& core, VkDeviceSize size, VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties);
	};
}
