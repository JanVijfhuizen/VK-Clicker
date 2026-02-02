#include "pch.h"
#include "Allocators.h"
#include "Vec.h"

namespace gr {
	PERS_Allocators::PERS_Allocators(Core& core, uint32_t uploadCapacity, uint32_t gpuCapacity)
	{
		auto builder = AllocatorBuilder();

		VkPhysicalDeviceMemoryProperties memProps;
		vkGetPhysicalDeviceMemoryProperties(core.physicalDevice, &memProps);

		_allocators = mem::Arr<Allocator>(PERS, memProps.memoryTypeCount);
		for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
			_allocators[i] = builder.Build(core, i, uploadCapacity);
	}
	void PERS_Allocators::Destroy()
	{
		OnScopeClear();
	}
	Memory PERS_Allocators::Alloc(uint32_t i, VkDeviceSize size, VkDeviceSize alignment)
	{
		return Get(i).Alloc(size, alignment);
	}
	Memory PERS_Allocators::Alloc(uint32_t i, VkBuffer buffer, bool bind)
	{
		return Get(i).Alloc(buffer, bind);
	}
	void PERS_Allocators::Clear(uint32_t i)
	{
		Get(i).Clear();
	}
	void PERS_Allocators::Clear()
	{
		for (uint32_t i = 0; i < _allocators.length(); i++)
			_allocators[i].Clear();
	}
	Allocator& PERS_Allocators::Get(uint32_t i)
	{
		return _allocators[i];
	}
	void PERS_Allocators::OnScopeClear()
	{
		for (uint32_t i = 0; i < _allocators.length(); i++)
			_allocators[i].Destroy();
	}
}