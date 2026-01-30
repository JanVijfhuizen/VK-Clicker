#pragma once

namespace gr {
	struct Memory final {
		VkDeviceMemory value;
		VkDeviceSize size;
		VkDeviceSize offset;
	};
}