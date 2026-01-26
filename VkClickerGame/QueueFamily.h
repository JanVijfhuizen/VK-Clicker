#pragma once

namespace gr {
#define QUEUE_LEN (int)QueueType::length

	enum class QueueType
	{
		graphics,
		transfer,
		compute,
		present,
		length
	};

	struct QueueFamily final {
		uint32_t queues[(int)QueueType::length];

		bool Complete();
	};
}
