#include "pch.h"
#include "Window.h"
#include "Core.h"
#include "SwapChain.h"
#include "PipelineBuilder.h"
#include "DescriptorSetLayoutBuilder.h"
#include "PushConstant.h"

struct Renderer final : public gr::SwapChainResource {
private:
    gr::Pipeline _pipeline;
    VkDescriptorSetLayout _descLayout;
    virtual void OnCreate(const gr::Core& core, gr::SwapChain& swapChain) {
        auto _ = mem::scope(TEMP);
        auto descLayoutBuilder = gr::TEMP_DescriptorSetLayoutBuilder();
        descLayoutBuilder.AddBinding(gr::BindingType::ubo, gr::BindingStep::fragment);
        _descLayout = descLayoutBuilder.Build(core);
        auto pipelineBuilder = gr::TEMP_PipelineBuilder();
        pipelineBuilder.AddLayout(_descLayout);
        pipelineBuilder.SetPushConstantSize(sizeof(gr::PushConstant));
        _pipeline = pipelineBuilder.Build(core, swapChain.GetRenderPass());
    }
    virtual void OnDestroy(const gr::Core& core, gr::SwapChain& swapChain) {
        _pipeline.Destroy(core);
        vkDestroyDescriptorSetLayout(core.device, _descLayout, nullptr);
    }
};

int main()
{
    mem::init();

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
        swapChain.Frame(window);
        mem::frame();
    }

    scope.clear();
    mem::end();
}