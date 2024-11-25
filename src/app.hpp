#pragma once

#include <memory>

#include "engine/vulkan/descriptors.hpp"
#include "engine/window/window.hpp"
#include "engine/vulkan/device.hpp"
#include "engine/vulkan/renderer.hpp"

namespace muon {

class App {
public:
    App(WindowProperties &properties);
    ~App();

    void run();
private:
    WindowProperties properties;
    Window window{properties};
    Device device{window};
    Renderer renderer{window, device};

    std::unique_ptr<DescriptorPool> global_pool;
};

}
