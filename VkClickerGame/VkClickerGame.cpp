#include "pch.h"
#include "Instance.h"
#include "Window.h"

int main()
{
    mem::init();

    auto windowBuilder = WindowBuilder();
    auto window = windowBuilder.SetName("My Engine").Build();

    auto instanceBuilder = InstanceBuilder();
    auto instance = instanceBuilder.SetName("My Engine").AddGLFWSupport().Build();

    while (window.Update()) {
        mem::frame();
    }

    window.Destroy();
    mem::end();
}