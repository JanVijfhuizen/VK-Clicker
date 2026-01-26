#pragma once
#include "Core.h"

namespace gr {
	struct RenderPassBuilder final
	{
		VkRenderPass Build(Core& core, VkFormat format);
	};
}