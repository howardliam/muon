#pragma once

#include "engine/window/window.hpp"

#include "engine/vulkan/device.hpp"
#include "engine/vulkan/renderer.hpp"

class App {
public:
    App(WindowProperties &properties);
    ~App();

    void run();
private:
    WindowProperties &properties;
    Window window{properties};
    Device device{window};
    Renderer renderer{window, device};
};