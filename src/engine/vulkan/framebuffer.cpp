#include "engine/vulkan/framebuffer.hpp"
#include <spdlog/spdlog.h>

namespace muon {

    Framebuffer::Framebuffer(Device &device, VkRenderPass render_pass, VkExtent2D extent) {
        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = VK_NULL_HANDLE;
        framebuffer_info.width = extent.width;
        framebuffer_info.height = extent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(device.getDevice(), &framebuffer_info, 0, &framebuffer) != VK_SUCCESS) {
            spdlog::error("Failed to create framebuffer");
        }
    }

}
