#include "pch.h"
#include "DescriptorSetLayoutManager.h"

namespace gr {
	DescriptorSetLayoutManager::DescriptorSetLayoutManager(ARENA arena, Core& core)
	{
		_arena = arena;
		_core = &core;
	}
	void DescriptorSetLayoutManager::OnScopeClear()
	{
		auto _ = mem::scope(TEMP);
		auto arr = _sets.arr(TEMP);
		for (int32_t i = arr.length() - 1; i >= 0; i--)
			vkDestroyDescriptorSetLayout(_core->device, arr[i].layout, nullptr);
	}
}