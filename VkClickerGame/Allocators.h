#pragma once
#include "Core.h"
#include "Allocator.h"

namespace gr {
	enum class AllocatorType {
		upload,
		gpu,
		length
	};

	struct Allocators final : public mem::IScoped {
		Allocators(Core& core, uint32_t uploadCapacity, uint32_t gpuCapacity);
		void Destroy();

		Memory Alloc(AllocatorType type, VkDeviceSize size, VkDeviceSize alignment);
		Memory Alloc(AllocatorType type, VkBuffer buffer, bool bind);
		void Clear(AllocatorType type);

	private:
		Allocator _allocators[(int)AllocatorType::length];

		Allocator& Get(AllocatorType type);
		virtual void OnScopeClear() override;
	};
}