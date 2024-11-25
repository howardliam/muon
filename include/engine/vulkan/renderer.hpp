#pragma once

#include "engine/vulkan/device.hpp"
#include "engine/window/window.hpp"
#include "engine/vulkan/swapchain.hpp"

namespace muon {

    class Renderer {
    public:
        Renderer(Window &window, Device &device);
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer& operator=(const Renderer &) = delete;

        VkCommandBuffer begin_frame();
        void end_frame();

        void begin_swapchain_render_pass(VkCommandBuffer command_buffer);
        void end_swapchain_render_pass(VkCommandBuffer command_buffer);

        VkRenderPass get_swapchain_render_pass() const { return swapchain->get_render_pass(); }
        VkCommandBuffer get_current_command_buffer() const { return command_buffers[current_frame_index]; }
        void set_clear_colour(VkClearColorValue new_clear) { clear_colour = new_clear; }
        void set_clear_depth_stencil(VkClearDepthStencilValue new_clear) { clear_depth_stencil = new_clear; }
        int get_frame_index() const { return current_frame_index; }
        bool is_frame_in_progress() const { return frame_in_progress; }
        float get_aspect_ratio() const { return swapchain->extent_aspect_ratio(); }

    private:
        Window &window;
        Device &device;
        std::unique_ptr<Swapchain> swapchain;
        std::vector<VkCommandBuffer> command_buffers;

        VkClearColorValue clear_colour{0.0f, 0.0f, 0.0f, 1.0f};
        VkClearDepthStencilValue clear_depth_stencil{1.0f, 0};

        uint32_t current_image_index{};
        int current_frame_index{0};
        bool frame_in_progress{false};

        void create_command_buffers();
        void free_command_buffers();
        void recreate_swapchain();
    };

}
