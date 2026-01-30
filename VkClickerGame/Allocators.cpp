#include "pch.h"
#include "Allocators.h"

namespace gr {
	Allocators::Allocators(Core& core, uint32_t uploadCapacity, uint32_t gpuCapacity)
	{
		auto builder = AllocatorBuilder();
		// WIP TYPE BITS.
		Get(AllocatorType::upload) = builder.Build(core, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT +
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uploadCapacity);
		Get(AllocatorType::gpu) = builder.Build(core, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uploadCapacity);
	}
	void Allocators::Destroy()
	{
		OnScopeClear();
	}
	Memory Allocators::Alloc(AllocatorType type, VkDeviceSize size, VkDeviceSize alignment)
	{
		return Get(type).Alloc(size, alignment);
	}
	Memory Allocators::Alloc(AllocatorType type, VkBuffer buffer, bool bind)
	{
		return Get(type).Alloc(buffer, bind);
	}
	void Allocators::Clear(AllocatorType type)
	{
		for (uint32_t i = 0; i < (int)AllocatorType::length; i++)
			_allocators[i].Clear();
	}
	Allocator& Allocators::Get(AllocatorType type)
	{
		return _allocators[(int)type];
	}
	void Allocators::OnScopeClear()
	{
		for (uint32_t i = 0; i < (int)AllocatorType::length; i++)
			_allocators[i].Destroy();
	}
}