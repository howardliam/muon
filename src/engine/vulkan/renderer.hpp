#pragma once

#include "device.hpp"
#include "../window/window.hpp"
#include "swapchain.hpp"

class Renderer {
public:
    Renderer(Window &window, Device &device);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    VkCommandBuffer begin_frame();
    void end_frame();

    void begin_swapchain_render_pass(VkCommandBuffer command_buffer);
    void end_swapchain_render_pass(VkCommandBuffer command_buffer);

    VkRenderPass get_swapchain_render_pass() const { return swapchain->get_render_pass(); }
    VkCommandBuffer get_current_command_buffer() const { return command_buffers[current_frame_index]; }
    int get_frame_index() const { return current_frame_index; }
    bool is_frame_in_progress() const { return frame_in_progress; }
    float get_aspect_ratio() const { return swapchain->extent_aspect_ratio(); }

private:
    Window &window;
    Device &device;
    std::unique_ptr<Swapchain> swapchain;
    std::vector<VkCommandBuffer> command_buffers;

    uint32_t current_image_index{};
    int current_frame_index{0};
    bool frame_in_progress{false};

    void create_command_buffers();
    void free_command_buffers();
    void recreate_swapchain();
};
