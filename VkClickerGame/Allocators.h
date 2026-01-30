#pragma once
#include "Core.h"
#include "Allocator.h"

namespace gr {
	enum class AllocatorType {
		upload,
		gpu
	};

	struct Allocators final : public mem::IScoped {
		Allocators(Core& core, uint32_t uploadCapacity, uint32_t gpuCapacity);
	private:
		Allocator _upload, _gpu;

		virtual void OnScopeClear() override;
	};
}