#include "pch.h"
#include "Window.h"
#include "Core.h"
#include "SwapChain.h"

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

    while (window.Update()) {
        swapChain.Frame(window);
        mem::frame();
    }

    scope.clear();
    mem::end();
}