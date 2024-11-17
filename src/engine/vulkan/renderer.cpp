#include "renderer.hpp"
#include <SDL3/SDL_events.h>

Renderer::Renderer(Window &window, Device &device) : window{window}, device{device} {
    recreate_swapchain();
    create_command_buffers();
}

Renderer::~Renderer() {
    free_command_buffers();
}

VkCommandBuffer Renderer::begin_frame() {
    const auto result = swapchain->acquire_next_image(&current_image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swapchain();
        return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        spdlog::error("Failed to acquire next swap chain image");
        exit(exitcode::FAILURE);
    }

    frame_in_progress = true;

    const auto command_buffer = get_current_command_buffer();

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
        spdlog::error("Failed to begin recording command buffer");
        exit(exitcode::FAILURE);
    }

    return command_buffer;
}

void Renderer::end_frame() {
    const auto command_buffer = get_current_command_buffer();

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
        spdlog::error("Failed to record command buffer");
        exit(exitcode::FAILURE);
    }

    const auto result = swapchain->submit_command_buffers(&command_buffer, &current_image_index);

    /* Resizing window */
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.was_resized()) {
        window.reset_resized();
        recreate_swapchain();
    } else if (result != VK_SUCCESS) {
        spdlog::error("Failed to present swapchain image");
        exit(exitcode::FAILURE);
    }

    frame_in_progress = false;
    current_frame_index = (current_frame_index + 1) % Swapchain::MAX_FRAMES_IN_FLIGHT;
}


void Renderer::begin_swapchain_render_pass(VkCommandBuffer command_buffer) {
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = swapchain->get_render_pass();
    render_pass_info.framebuffer = swapchain->get_frame_buffer(current_image_index);

    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = swapchain->get_swapchain_extent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount = static_cast<uint32_t>(clearValues.size());
    render_pass_info.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain->get_swapchain_extent().width);
    viewport.height = static_cast<float>(swapchain->get_swapchain_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain->get_swapchain_extent();

    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

void Renderer::end_swapchain_render_pass(VkCommandBuffer command_buffer) {
    vkCmdEndRenderPass(command_buffer);
}

void Renderer::create_command_buffers() {
    command_buffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = device.get_command_pool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

    if (vkAllocateCommandBuffers(device.get_device(), &allocInfo, command_buffers.data()) != VK_SUCCESS) {
        spdlog::error("Failed to allocate command buffers");
        exit(exitcode::FAILURE);
    }
}

void Renderer::free_command_buffers() {
    vkFreeCommandBuffers(device.get_device(), device.get_command_pool(), static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
    command_buffers.clear();
}

void Renderer::recreate_swapchain() {
    auto extent = window.get_extent();
    while (extent.width == 0 || extent.height == 0) {
        extent = window.get_extent();
        SDL_Event event;
        SDL_WaitEvent(&event);
    }

    vkDeviceWaitIdle(device.get_device());

    if (swapchain == nullptr) {
        swapchain = std::make_unique<Swapchain>(device, extent);
    } else {
        std::shared_ptr oldSwapchain = std::move(swapchain);
        swapchain = std::make_unique<Swapchain>(device, extent, oldSwapchain);
        if (!swapchain->compare_swap_formats(*oldSwapchain)) {
            spdlog::error("Swapchain does not match swap formats");
            exit(exitcode::FAILURE);
        }
    }
}
