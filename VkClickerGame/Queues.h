#pragma once

namespace gr {
	struct Queues final {
		enum Type
		{
			graphics,
			transfer,
			compute,
			present,
			length
		};

		uint32_t queues[Type::length];

		bool Equal(uint32_t a, uint32_t b);
		bool Complete();
	};
}
