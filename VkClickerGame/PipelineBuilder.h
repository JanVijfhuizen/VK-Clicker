#pragma once
#include "Vertex.h"
#include "Core.h"

namespace gr {
	struct Pipeline final {
		VkPipeline value;
		VkPipelineLayout layout;
		void Destroy(Core& core);
	};

	struct TEMP_PipelineBuilder final {
		Pipeline Build(Core& core, VkRenderPass renderPass);
		TEMP_PipelineBuilder& SetVertPath(const char* path);
		TEMP_PipelineBuilder& SetFragPath(const char* path);
		TEMP_PipelineBuilder& AddLayout(VkDescriptorSetLayout layout);
		TEMP_PipelineBuilder& SetPushConstantSize(uint32_t size);
	private:
		mem::Link<VkDescriptorSetLayout> _layouts;
		uint32_t _pushConstantSize = 0;

		const char* _vertPath = "vert.spv";
		const char* _fragPath = "frag.spv";
	};
}
