#include "pch.h"
#include "Mesh.h"

namespace gr {
    TEMP_MeshBuilder::TEMP_MeshBuilder()
    {
        SetQuad();
    }
    Mesh TEMP_MeshBuilder::Build(const Core& core, SwapChain& swapChain)
    {
        Mesh mesh{};

        VkCommandBuffer cmd;
        swapChain.AllocCommandBuffers(gr::Queues::graphics, 1, &cmd); 

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(cmd, &beginInfo);

        const size_t vertSize = sizeof(Vertex) * _vertices.length();
        const size_t indSize = sizeof(Ind) * _indices.length();

        auto builder = gr::BufferBuilder();
        auto vertStaging = builder.Build(
            core,
            vertSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        builder = gr::BufferBuilder();
        auto vertBuffer = builder.Build(
            core,
            vertSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        builder = gr::BufferBuilder();
        auto indStaging = builder.Build(
            core,
            indSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        builder = gr::BufferBuilder();
        auto indBuffer = builder.Build(
            core,
            indSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        void* data;
        vkMapMemory(core.device, vertStaging.memory, 0, VK_WHOLE_SIZE, 0, &data);
        memcpy(data, _vertices.ptr(), vertSize);
        vkUnmapMemory(core.device, vertStaging.memory);

        vkMapMemory(core.device, indStaging.memory, 0, VK_WHOLE_SIZE, 0, &data);
        memcpy(data, _indices.ptr(), indSize);
        vkUnmapMemory(core.device, indStaging.memory);

        VkBufferCopy copy{};
        copy.size = vertSize;
        vkCmdCopyBuffer(cmd, vertStaging.value, vertBuffer.value, 1, &copy);

        copy.size = indSize;
        vkCmdCopyBuffer(cmd, indStaging.value, indBuffer.value, 1, &copy);

        VkBufferMemoryBarrier barriers[2]{};

        barriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barriers[0].dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        barriers[0].buffer = vertBuffer.value;
        barriers[0].offset = 0;
        barriers[0].size = VK_WHOLE_SIZE;

        barriers[1].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barriers[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barriers[1].dstAccessMask = VK_ACCESS_INDEX_READ_BIT;
        barriers[1].buffer = indBuffer.value;
        barriers[1].offset = 0;
        barriers[1].size = VK_WHOLE_SIZE;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
            0,
            0, nullptr,
            2, barriers,
            0, nullptr
        );

        vkEndCommandBuffer(cmd);

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd;

        VkFence fence;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = 0;
        vkCreateFence(core.device, &fenceInfo, nullptr, &fence);

        vkQueueSubmit(core.queues[Queues::graphics], 1, &submit, fence);
        vkWaitForFences(core.device, 1, &fence, VK_TRUE, UINT64_MAX);
        vkDestroyFence(core.device, fence, nullptr);

        vertStaging.Destroy(core);
        indStaging.Destroy(core);

        mesh.vertexBuffer = vertBuffer;
        mesh.indexBuffer = indBuffer;
        mesh.indexCount = _indices.length();
        return mesh;
    }
    TEMP_MeshBuilder& TEMP_MeshBuilder::SetTriangle()
    {
        Vertex vertices[] = {
            {{  0.0f, -0.5f }},
            {{  0.5f,  0.5f }},
            {{ -0.5f,  0.5f }}
        };

        _vertices = mem::Arr<Vertex>(TEMP, 3);
        _vertices.put(0, vertices, 3);

        uint16_t indices[] = {
            0, 1, 2
        };

        _indices = mem::Arr<Ind>(TEMP, 3);
        _indices.put(0, indices, 3);
        return *this;
    }
    TEMP_MeshBuilder& TEMP_MeshBuilder::SetQuad()
    {
        Vertex vertices[] = {
            {{ -0.5f, -0.5f }},
            {{  0.5f, -0.5f }},
            {{  0.5f,  0.5f }},
            {{ -0.5f,  0.5f }}
        };

        _vertices = mem::Arr<Vertex>(TEMP, 4);
        _vertices.put(0, vertices, 4);

        Ind indices[] = {
            0,1,2,
            2,3,0
        };

        _indices = mem::Arr<Ind>(TEMP, 6);
        _indices.put(0, indices, 6);
        return *this;
    }
    void Mesh::Draw(VkCommandBuffer cmd, const Core& core)
    {
        VkBuffer vertexBuffers[] = { vertexBuffer.value };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(
            cmd,
            indexBuffer.value,
            0,
            VK_INDEX_TYPE_UINT16
        );

        vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
    }
    void Mesh::Destroy(const Core& core)
    {
        indexBuffer.Destroy(core);
        vertexBuffer.Destroy(core);
    }
}