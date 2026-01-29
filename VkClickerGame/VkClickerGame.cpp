#include "pch.h"
#include "Window.h"
#include "Core.h"
#include "SwapChain.h"
#include "PipelineBuilder.h"
#include "DescriptorSetLayoutBuilder.h"
#include "DescriptorSetLayoutManager.h"
#include "PushConstant.h"
#include "VkCheck.h"
#include "ColorUBO.h"
#include "Buffer.h"
#include "Mesh.h"

struct Renderer final : public gr::SwapChainResource {
    void Init(const gr::Core& core, gr::SwapChain& swapChain, gr::DescriptorSetLayoutManager& descLayoutManager) {
        _descLayoutManager = &descLayoutManager;
        auto _ = mem::scope(TEMP);
        auto meshBuilder = gr::TEMP_MeshBuilder();
        _mesh = meshBuilder.SetQuad().Build(core, swapChain);
    }
    void Exit(const gr::Core& core) {
        _mesh.Destroy(core);
    }
    void Draw(const gr::Core& core, gr::SwapChain& swapChain) {
        auto cmd = swapChain.GetCmd();
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline.value);

        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            _pipeline.layout,
            0,
            1, &_sets[swapChain.GetIndex()],
            0, nullptr
        );

        gr::PushConstant pc{};
        Rotate(pc);
        Color(core, swapChain);

        vkCmdPushConstants(
            cmd,
            _pipeline.layout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(gr::PushConstant),
            &pc
        );

        _mesh.Draw(cmd, core);

        pc.transform = glm::mat4x4(1);
        vkCmdPushConstants(
            cmd,
            _pipeline.layout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(gr::PushConstant),
            &pc
        );

        _mesh.Draw(cmd, core);
    }

    void Rotate(gr::PushConstant& pc) {
        static float angle = 0.0f;
        angle += 0.001f;

        glm::mat4 model = glm::rotate(
            glm::mat4(1.0f),
            angle,
            glm::vec3(0.0f, 0.0f, 1.0f)
        );

        glm::mat4 proj = glm::ortho(
            -1.0f, 1.0f,
            -1.0f, 1.0f
        );

        pc.transform = proj * model;
    }

    void Color(const gr::Core& core, gr::SwapChain& swapChain) {
        static float t = 0;
        t += .001f;

        gr::ColorUBO ubo{};
        ubo.color = {
            0.5f + 0.5f * sin(t),
            0.5f + 0.5f * sin(t + 2.094f),
            0.5f + 0.5f * sin(t + 4.188f)
        };

        void* data;
        vkMapMemory(
            core.device,
            _buffers[swapChain.GetIndex()].memory,
            0,
            sizeof(gr::ColorUBO),
            0,
            &data
        );

        memcpy(data, &ubo, sizeof(gr::ColorUBO));
        vkUnmapMemory(core.device, _buffers[swapChain.GetIndex()].memory);
    }

private:
    gr::DescriptorSetLayoutManager* _descLayoutManager;
    gr::Pipeline _pipeline;
    VkDescriptorSetLayout _descLayout;
    VkDescriptorPool _descriptorPool;
    mem::Scope _scope;
    mem::Arr<VkDescriptorSet> _sets;
    mem::Arr<gr::Buffer> _buffers;
    gr::Mesh _mesh;

    virtual void OnCreate(const gr::Core& core, gr::SwapChain& swapChain) {
        auto _ = mem::scope(TEMP);
        auto descLayoutBuilder = gr::TEMP_DescriptorSetLayoutBuilder();
        descLayoutBuilder.AddBinding(gr::BindingType::ubo, gr::BindingStep::fragment);
        _descLayout = descLayoutBuilder.Build(core, *_descLayoutManager);
        auto pipelineBuilder = gr::TEMP_PipelineBuilder();
        pipelineBuilder.AddLayout(_descLayout);
        pipelineBuilder.SetPushConstantSize(sizeof(gr::PushConstant));
        _pipeline = pipelineBuilder.Build(core, swapChain.GetRenderPass());

        auto frameCount = swapChain.GetFrameCount();

        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = frameCount;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = frameCount;

        gr::VkCheck(vkCreateDescriptorPool(core.device, &poolInfo, nullptr, &_descriptorPool));

        _scope = mem::scope(PERS + 1);
        auto layouts = mem::Arr<VkDescriptorSetLayout>(TEMP, frameCount);
        layouts.set(_descLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = _descriptorPool;
        allocInfo.descriptorSetCount = layouts.length();
        allocInfo.pSetLayouts = layouts.ptr();

        _sets = mem::Arr<VkDescriptorSet>(PERS + 1, frameCount);
        gr::VkCheck(vkAllocateDescriptorSets(core.device, &allocInfo, _sets.ptr()));

        _buffers = mem::Arr<gr::Buffer>(PERS + 1, frameCount);
        for (uint32_t i = 0; i < frameCount; i++) {
            auto builder = gr::BufferBuilder();
            _buffers[i] = builder.Build(core, sizeof(gr::ColorUBO),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }

        for (uint32_t i = 0; i < frameCount; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = _buffers[i].value;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(gr::ColorUBO);

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = _sets[i];
            write.dstBinding = 0;
            write.dstArrayElement = 0;
            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write.descriptorCount = 1;
            write.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(
                core.device,
                1,
                &write,
                0,
                nullptr
            );
        }
    }
    virtual void OnDestroy(const gr::Core& core, gr::SwapChain& swapChain) {
        for (uint32_t i = 0; i < _buffers.length(); i++)
            _buffers[i].Destroy(core);
        vkDestroyDescriptorPool(core.device, _descriptorPool, nullptr);
        _pipeline.Destroy(core);
        _scope.clear();
    }
};

int main()
{
    mem::Info info{};
    info.persistentLength = 2;
    mem::init(info);

    auto scope = mem::manualScope(PERS);

    auto windowBuilder = gr::WindowBuilder();
    auto window = windowBuilder.SetName("My Engine").SetResolution({800, 600}).Build();
    scope.bind(window);

    auto coreBuilder = gr::CoreBuilder();
    auto core = coreBuilder.AddGLFWSupport().EnableValidationLayers(true).Build(PERS, window);

    auto swapChainBuilder = gr::SwapChainBuilder();
    auto swapChain = swapChainBuilder.Build(PERS, core, window);

    auto descLayoutManager = gr::DescriptorSetLayoutManager(PERS, core);

    scope.bind(core);
    scope.bind(swapChain);
    scope.bind(descLayoutManager);

    auto renderer = Renderer();
    renderer.Init(core, swapChain, descLayoutManager);
    swapChain.BindResource(&renderer);

    while (window.Update()) {
        renderer.Draw(core, swapChain);
        swapChain.Frame(window);
        mem::frame();
    }

    renderer.Exit(core);

    scope.clear();
    mem::end();
}