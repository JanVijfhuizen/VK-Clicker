#include "pch.h"
#include "VkCheck.h"

namespace gr {
	bool VkCheck(int32_t result)
	{
		assert(result == VK_SUCCESS);
		return result == 0;
	}
}