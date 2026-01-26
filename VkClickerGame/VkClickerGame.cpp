#include "pch.h"
#include "Window.h"
#include "Core.h"
#include "SwapChain.h"
#include "PipelineBuilder.h"
#include "DescriptorSetLayoutBuilder.h"
#include "PushConstant.h"

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

    gr::Pipeline pipeline;
    VkDescriptorSetLayout descLayout;
    {
        auto _ = mem::scope(TEMP);
        auto descLayoutBuilder = gr::TEMP_DescriptorSetLayoutBuilder();
        descLayoutBuilder.AddBinding(gr::BindingType::ubo, gr::BindingStep::fragment);
        descLayout = descLayoutBuilder.Build(core);
        auto pipelineBuilder = gr::TEMP_PipelineBuilder();
        pipelineBuilder.AddLayout(descLayout);
        pipelineBuilder.SetPushConstantSize(sizeof(gr::PushConstant));
        pipeline = pipelineBuilder.Build(core, swapChain.GetRenderPass());
    }

    while (window.Update()) {
        swapChain.Frame(window);
        mem::frame();
    }

    pipeline.Destroy(core);

    scope.clear();
    mem::end();
}