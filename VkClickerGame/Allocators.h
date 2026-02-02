#pragma once
#include "Core.h"
#include "Allocator.h"

namespace gr {
	// Pretty bad allocator but I dont want to spend too much time on it right now.
	// Allocates all available pools equally with no regard which ones are used more frequently and cannot resize.
	struct PERS_Allocators final : public mem::IScoped {
		PERS_Allocators(Core& core, uint32_t uploadCapacity, uint32_t gpuCapacity);
		void Destroy();

		Memory Alloc(uint32_t i, VkDeviceSize size, VkDeviceSize alignment);
		Memory Alloc(uint32_t i, VkBuffer buffer, bool bind);
		void Clear(uint32_t i);
		void Clear();

	private:
		mem::Arr<Allocator> _allocators;

		Allocator& Get(uint32_t i);
		virtual void OnScopeClear() override;
	};
}