#include "engine/vulkan/renderer.hpp"

#include <SDL3/SDL_events.h>

namespace muon {

    Renderer::Renderer(Window &window, Device &device) : window{window}, device{device} {
        recreateSwapchain();
        createCommandBuffers();
    }

    Renderer::~Renderer() {
        freeCommandBuffers();
    }

    VkCommandBuffer Renderer::beginFrame() {
        const auto result = swapchain->acquireNextImage(&current_image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapchain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            spdlog::error("Failed to acquire next swap chain image");
            exit(exitcode::FAILURE);
        }

        frame_in_progress = true;

        const auto command_buffer = getCurrentCommandBuffer();

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
            spdlog::error("Failed to begin recording command buffer");
            exit(exitcode::FAILURE);
        }

        return command_buffer;
    }

    void Renderer::endFrame() {
        const auto command_buffer = getCurrentCommandBuffer();

        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
            spdlog::error("Failed to record command buffer");
            exit(exitcode::FAILURE);
        }

        const auto result = swapchain->submitCommandBuffers(&command_buffer, &current_image_index);

        /* Resizing window */
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasResized()) {
            window.resetResized();
            recreateSwapchain();
        } else if (result != VK_SUCCESS) {
            spdlog::error("Failed to present swapchain image");
            exit(exitcode::FAILURE);
        }

        frame_in_progress = false;
        current_frame_index = (current_frame_index + 1) % Swapchain::MAX_FRAMES_IN_FLIGHT;
    }


    void Renderer::beginSwapchainRenderPass(VkCommandBuffer command_buffer) {
        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = swapchain->getRenderPass();
        render_pass_info.framebuffer = swapchain->getFramebuffer(current_image_index);

        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = swapchain->getSwapchainExtent();

        std::array<VkClearValue, 2> clear_values{};
        clear_values[0].color = clear_color;
        clear_values[1].depthStencil = clear_depth_stencil;

        render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapchain->getSwapchainExtent().width);
        viewport.height = static_cast<float>(swapchain->getSwapchainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapchain->getSwapchainExtent();

        vkCmdSetViewport(command_buffer, 0, 1, &viewport);
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    }

    void Renderer::endSwapchainRenderPass(VkCommandBuffer command_buffer) {
        vkCmdEndRenderPass(command_buffer);
    }

    void Renderer::createCommandBuffers() {
        command_buffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = device.getCommandPool();
        alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

        if (vkAllocateCommandBuffers(device.getDevice(), &alloc_info, command_buffers.data()) != VK_SUCCESS) {
            spdlog::error("Failed to allocate command buffers");
            exit(exitcode::FAILURE);
        }
    }

    void Renderer::freeCommandBuffers() {
        vkFreeCommandBuffers(device.getDevice(), device.getCommandPool(), static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
        command_buffers.clear();
    }

    void Renderer::recreateSwapchain() {
        auto extent = window.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = window.getExtent();
            SDL_Event event;
            SDL_WaitEvent(&event);
        }

        vkDeviceWaitIdle(device.getDevice());

        if (swapchain == nullptr) {
            swapchain = std::make_unique<Swapchain>(device, extent);
        } else {
            std::shared_ptr old_swap_chain = std::move(swapchain);
            swapchain = std::make_unique<Swapchain>(device, extent, old_swap_chain);
            if (!swapchain->compareSwapFormats(*old_swap_chain)) {
                spdlog::error("Swapchain does not match swap formats");
                exit(exitcode::FAILURE);
            }
        }
    }

}
