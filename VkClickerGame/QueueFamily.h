#pragma once

namespace gr {
	struct QueueFamily final {
		union
		{
			uint32_t queues[4]{ UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
			struct
			{
				uint32_t graphics;
				uint32_t present;
				uint32_t transfer;
				uint32_t compute;
			};
		};

		bool Complete();
	};
}
