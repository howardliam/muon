#include "engine/vulkan/framebuffer.hpp"
#include <spdlog/spdlog.h>

namespace muon {

    Framebuffer::Framebuffer(Device &device, vk::RenderPass render_pass, vk::Extent2D extent) {
        vk::FramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = vk::StructureType::eFramebufferCreateInfo;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = nullptr;
        framebuffer_info.width = extent.width;
        framebuffer_info.height = extent.height;
        framebuffer_info.layers = 1;

        // if (device.getDevice().createFramebuffer(&framebuffer_info, 0, &framebuffer) != vk::Result::eSuccess) {
        //     spdlog::error("Failed to create framebuffer");
        // }
    }

}
