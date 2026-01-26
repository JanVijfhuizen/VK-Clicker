#pragma once
#include "glm/glm.hpp"

namespace gr {
	struct ColorUBO final {
		alignas(16) glm::vec3 color;
	};
}