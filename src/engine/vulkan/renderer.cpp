#include "engine/vulkan/renderer.hpp"

#include <SDL3/SDL_events.h>
#include <vulkan/vulkan_core.h>

namespace muon {

    Renderer::Renderer(Window &window, Device &device) : window{window}, device{device} {
        recreateSwapchain();
        createCommandBuffers();
    }

    Renderer::~Renderer() {
        freeCommandBuffers();
    }

    vk::CommandBuffer Renderer::beginFrame() {
        const vk::Result result = swapchain->acquireNextImage(&current_image_index);

        if (result == vk::Result::eErrorOutOfDateKHR) {
            recreateSwapchain();
            return nullptr;
        }

        if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
            spdlog::error("Failed to acquire next swap chain image");
            exit(exitcode::FAILURE);
        }

        frame_in_progress = true;

        const auto command_buffer = getCurrentCommandBuffer();

        vk::CommandBufferBeginInfo begin_info{};
        begin_info.sType = vk::StructureType::eCommandBufferBeginInfo;

        if (command_buffer.begin(&begin_info) != vk::Result::eSuccess) {
            spdlog::error("Failed to begin recording command buffer");
            exit(exitcode::FAILURE);
        }

        return command_buffer;
    }

    void Renderer::endFrame() {
        const auto command_buffer = getCurrentCommandBuffer();

        command_buffer.end();

        const auto result = swapchain->submitCommandBuffers(&command_buffer, &current_image_index);

        /* Resizing window */
        if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || window.wasResized()) {
            window.resetResized();
            recreateSwapchain();
        } else if (result != vk::Result::eSuccess) {
            spdlog::error("Failed to present swapchain image");
            exit(exitcode::FAILURE);
        }

        frame_in_progress = false;
        current_frame_index = (current_frame_index + 1) % Swapchain::MAX_FRAMES_IN_FLIGHT;
    }


    void Renderer::beginSwapchainRenderPass(vk::CommandBuffer command_buffer) {
        vk::RenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = vk::StructureType::eRenderPassBeginInfo;
        render_pass_info.renderPass = swapchain->getRenderPass();
        render_pass_info.framebuffer = swapchain->getFramebuffer(current_image_index);

        render_pass_info.renderArea.setOffset({0, 0});
        render_pass_info.renderArea.extent = swapchain->getSwapchainExtent();

        std::array<vk::ClearValue, 2> clear_values{};
        clear_values[0].color = clear_color;
        clear_values[1].depthStencil = clear_depth_stencil;

        render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_info.pClearValues = clear_values.data();

        command_buffer.beginRenderPass(&render_pass_info, vk::SubpassContents::eInline);

        vk::Viewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapchain->getSwapchainExtent().width);
        viewport.height = static_cast<float>(swapchain->getSwapchainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vk::Rect2D scissor{};
        scissor.setOffset({0, 0});
        scissor.extent = swapchain->getSwapchainExtent();

        command_buffer.setViewport(0, 1, &viewport);
        command_buffer.setScissor(0, 1, &scissor);
    }

    void Renderer::endSwapchainRenderPass(vk::CommandBuffer command_buffer) {
        command_buffer.endRenderPass();
    }

    void Renderer::createCommandBuffers() {
        command_buffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

        vk::CommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = vk::StructureType::eCommandBufferAllocateInfo;
        alloc_info.level = vk::CommandBufferLevel::ePrimary;
        alloc_info.commandPool = device.getCommandPool();
        alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

        if (device.getDevice().allocateCommandBuffers(&alloc_info, command_buffers.data()) != vk::Result::eSuccess) {
            spdlog::error("Failed to allocate command buffers");
            exit(exitcode::FAILURE);
        }
    }

    void Renderer::freeCommandBuffers() {
        device.getDevice().freeCommandBuffers(device.getCommandPool(), static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
        command_buffers.clear();
    }

    void Renderer::recreateSwapchain() {
        auto extent = window.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = window.getExtent();
            SDL_Event event;
            SDL_WaitEvent(&event);
        }

        device.getDevice().waitIdle();

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
