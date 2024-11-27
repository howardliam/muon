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

        VkCommandBuffer beginFrame();
        void endFrame();

        void beginSwapchainRenderPass(VkCommandBuffer command_buffer);
        void endSwapchainRenderPass(VkCommandBuffer command_buffer);

        VkRenderPass getSwapchainRenderPass() const { return swapchain->getRenderPass(); }
        VkCommandBuffer getCurrentCommandBuffer() const { return command_buffers[current_frame_index]; }
        void setClearColor(VkClearColorValue new_color) { clear_color = new_color; }
        void setClearDepthStencil(VkClearDepthStencilValue new_depth) { clear_depth_stencil = new_depth; }
        int getFrameIndex() const { return current_frame_index; }
        bool isFrameInProgress() const { return frame_in_progress; }
        float getAspectRatio() const { return swapchain->extentAspectRatio(); }

    private:
        Window &window;
        Device &device;
        std::unique_ptr<Swapchain> swapchain;
        std::vector<VkCommandBuffer> command_buffers;

        VkClearColorValue clear_color{0.0f, 0.0f, 0.0f, 1.0f};
        VkClearDepthStencilValue clear_depth_stencil{1.0f, 0};

        uint32_t current_image_index{};
        int current_frame_index{0};
        bool frame_in_progress{false};

        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapchain();
    };

}
