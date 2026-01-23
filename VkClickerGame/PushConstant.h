#pragma once
#include <glm/glm.hpp>

namespace gr {
	struct PushConstant final
	{
		glm::mat4x4 transform{};
	};
}