#pragma once

namespace mem
{
	struct ThreadPoolInfo final
	{
		uint32_t taskCapacity = 128;
	};

	struct ThreadPoolTask final
	{
		void (*func)(void* userPtr, uint32_t id, uint32_t mId);
		void (*callback)(void* userPtr, uint32_t mId) = nullptr;
		void* userPtr = nullptr;
		uint32_t mId = -1;
	};

	void p_initThreadPool(const ThreadPoolInfo& info);
	void destroyThreadPool();
	void threadPoolUpdate();
	void addThreadPoolTask(const ThreadPoolTask& task);
	uint32_t getThreadCapacity();
}


