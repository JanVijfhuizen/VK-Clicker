#include "pch.h"
#include "ThreadPool.h"
#include "Queue.h"
#include <thread>
#include <mutex>

namespace mem 
{
	struct ThreadPool final
	{
		Queue<ThreadPoolTask> open;
		Vec<ThreadPoolTask> closed;
		Arr<std::thread> threads;
		std::mutex mutex{};
		std::condition_variable cv{};
		bool quit = false;
		bool init = false;
	} pool{};

	void updateThread(const uint32_t id)
	{
		while (true)
		{
			ThreadPoolTask task;
			{
				std::unique_lock<std::mutex> lock(pool.mutex);
				pool.cv.wait(lock, [] {
					return pool.quit || pool.open.count() > 0;
					});

				if (pool.quit && pool.open.count() == 0)
					break;

				task = pool.open.pop();
			}
			
			task.func(task.userPtr, id, task.mId);

			std::unique_lock<std::mutex> lock(pool.mutex);
			pool.closed.add() = task;
		}
	}

	void p_initThreadPool(const ThreadPoolInfo& info)
	{
		assert(!pool.init);
		pool.init = true;

		pool.open = { PERS, info.taskCapacity };
		pool.closed = { PERS, info.taskCapacity };
		pool.threads = {PERS, std::thread::hardware_concurrency() };
		pool.threads.iter([](auto& thread, auto i) {
			thread = std::thread(updateThread, i);
			});
	}
	void destroyThreadPool()
	{
		assert(pool.init);
		{
			std::unique_lock<std::mutex> lock(pool.mutex);
			pool.quit = true;
		}
		pool.cv.notify_all();
		pool.init = false;
		pool.threads.iter([](auto& thread, auto) {
			thread.join();
			});
	}
	void threadPoolUpdate()
	{
		std::unique_lock<std::mutex> lock(pool.mutex);
		pool.closed.arr().iter([](ThreadPoolTask& task, auto) {
			if(task.callback)
				task.callback(task.userPtr, task.mId);
			});
		pool.closed.clear();
	}
	void addThreadPoolTask(const ThreadPoolTask& task)
	{
		{
			std::unique_lock<std::mutex> lock(pool.mutex);
			pool.open.add() = task;
		}
		pool.cv.notify_one();
	}
	uint32_t getThreadCapacity()
	{
		return std::thread::hardware_concurrency();
	}
}