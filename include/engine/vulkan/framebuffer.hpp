#pragma once

#include <vulkan/vulkan.hpp>

#include "engine/vulkan/device.hpp"

namespace muon {

    class Framebuffer {
    public:
        Framebuffer(Device &device, vk::RenderPass render_pass, vk::Extent2D extent);
        ~Framebuffer();

    private:
        vk::Framebuffer framebuffer;

    };

}
