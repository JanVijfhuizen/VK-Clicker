#pragma once

namespace gr {
	struct QueueFamily final {
		enum class Type
		{
			graphics,
			transfer,
			compute,
			present,
			length
		};

		uint32_t queues[(int)Type::length];

		bool Complete();
	};
}
