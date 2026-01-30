#include "pch.h"
#include "Allocators.h"

namespace gr {
	Allocators::Allocators(Core& core, uint32_t uploadCapacity, uint32_t gpuCapacity)
	{
		auto builder = AllocatorBuilder();
		_upload = builder.Build(core, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT + VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uploadCapacity);
		_gpu = builder.Build(core, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uploadCapacity);
	}
	void Allocators::OnScopeClear()
	{
		_upload.Destroy();
		_gpu.Destroy();
	}
}