#include "pch.h"
#include "Instance.h"
#include "Window.h"

int main()
{
    mem::init();

    auto scope = mem::manualScope(PERS);

    auto windowBuilder = WindowBuilder();
    auto window = windowBuilder.SetName("My Engine").SetResolution({800, 600}).Build();
    scope.bind(window);

    auto instanceBuilder = InstanceBuilder();
    auto instance = instanceBuilder.SetName("My Engine").AddGLFWSupport().SetPreferredPresentMode(PresentMode::mailbox).Build(PERS, window);
    scope.bind(instance);

    while (window.Update()) {
        mem::frame();
    }

    scope.clear();
    mem::end();
}