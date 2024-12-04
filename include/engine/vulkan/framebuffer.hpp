#pragma once

#include <vulkan/vulkan.hpp>

#include "engine/vulkan/device.hpp"

namespace muon {

    class Framebuffer {
    public:
        Framebuffer(Device &device, VkRenderPass render_pass, VkExtent2D extent);
        ~Framebuffer();

    private:
        VkFramebuffer framebuffer;

    };

}
