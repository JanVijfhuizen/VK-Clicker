#include "pch.h"
#include "Instance.h"
#include "Window.h"

int main()
{
    mem::init();

    auto scope = mem::manualScope(PERS);

    auto windowBuilder = gr::WindowBuilder();
    auto window = windowBuilder.SetName("My Engine").SetResolution({800, 600}).Build();
    scope.bind(window);

    auto instanceBuilder = gr::InstanceBuilder();
    auto instance = instanceBuilder.SetName("My Engine").AddGLFWSupport().SetPreferredPresentMode(gr::PresentMode::mailbox).Build(PERS, window);
    scope.bind(instance);

    while (window.Update()) {
        mem::frame();
    }

    scope.clear();
    mem::end();
}