#pragma once
#include "Buffer.h"
#include "Vertex.h"
#include "SwapChain.h"

namespace gr {
	struct Mesh final
	{
		Buffer vertexBuffer;
		Buffer indexBuffer;
		uint32_t indexCount;

		void Draw(VkCommandBuffer cmd, const Core& core);
		void Destroy(const Core& core);
	};

	struct TEMP_MeshBuilder final {
		TEMP_MeshBuilder();
		Mesh Build(const Core& core, SwapChain& swapChain);
		TEMP_MeshBuilder& SetTriangle();
		TEMP_MeshBuilder& SetQuad();
	private:
		mem::Arr<Vertex> _vertices;
		mem::Arr<Ind> _indices{};
	};
}
