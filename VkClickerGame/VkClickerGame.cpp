#include "pch.h"
#include "Window.h"
#include "Core.h"
#include "SwapChain.h"
#include "PipelineBuilder.h"
#include "DescriptorSetLayoutBuilder.h"
#include "PushConstant.h"
#include "VkCheck.h"
#include "ColorUBO.h"
#include "BufferBuilder.h"

struct Renderer final : public gr::SwapChainResource {
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

        //TODO
        /*
        VkBuffer vertexBuffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
        */
        gr::PushConstant pc{};

        vkCmdPushConstants(
            cmd,
            _pipeline.layout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(gr::PushConstant),
            &pc
        );

        vkCmdDraw(cmd, 3, 1, 0, 0);
    }

private:
    gr::Pipeline _pipeline;
    VkDescriptorSetLayout _descLayout;
    VkDescriptorPool _descriptorPool;
    mem::Scope _scope;
    mem::Arr<VkDescriptorSet> _sets;
    mem::Arr<gr::Buffer> _buffers;

    virtual void OnCreate(const gr::Core& core, gr::SwapChain& swapChain) {
        auto _ = mem::scope(TEMP);
        auto descLayoutBuilder = gr::TEMP_DescriptorSetLayoutBuilder();
        descLayoutBuilder.AddBinding(gr::BindingType::ubo, gr::BindingStep::fragment);
        _descLayout = descLayoutBuilder.Build(core);
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
    }
    virtual void OnDestroy(const gr::Core& core, gr::SwapChain& swapChain) {
        for (uint32_t i = 0; i < _buffers.length(); i++)
            _buffers[i].Destroy(core);
        vkDestroyDescriptorPool(core.device, _descriptorPool, nullptr);
        _pipeline.Destroy(core);
        vkDestroyDescriptorSetLayout(core.device, _descLayout, nullptr);
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
    auto core = coreBuilder.AddGLFWSupport().Build(PERS, window);

    auto swapChainBuilder = gr::SwapChainBuilder();
    auto swapChain = swapChainBuilder.Build(PERS, core, window);

    scope.bind(core);
    scope.bind(swapChain);

    auto renderer = Renderer();
    swapChain.BindResource(&renderer);

    while (window.Update()) {
        renderer.Draw(core, swapChain);
        swapChain.Frame(window);
        mem::frame();
    }

    scope.clear();
    mem::end();
}