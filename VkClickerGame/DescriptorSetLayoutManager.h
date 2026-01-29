#pragma once
#include "DescriptorSetLayoutBuilder.h"

namespace gr {
	struct DescriptorSetLayoutManager final : public mem::IScoped
	{
		friend struct TEMP_DescriptorSetLayoutBuilder;
		DescriptorSetLayoutManager(ARENA arena, Core& core);

	private:
		struct Set final {
			mem::Arr<uint64_t> keys;
			VkDescriptorSetLayout layout;
		};

		ARENA _arena;
		Core* _core;
		mem::Link<Set> _sets;

		virtual void OnScopeClear() override;
	};


}