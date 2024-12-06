#pragma once

#include <vector>
#include <memory>

#include <vulkan/vulkan.hpp>

#include "engine/vulkan/device.hpp"

namespace muon {

    class Swapchain {
    public:
        static constexpr int32_t MAX_FRAMES_IN_FLIGHT = 2;

        Swapchain(Device &device, vk::Extent2D window_extent);
        Swapchain(Device &device, vk::Extent2D window_extent, std::shared_ptr<Swapchain> previous);
        ~Swapchain();

        Swapchain(const Swapchain &) = delete;
        Swapchain& operator=(const Swapchain &) = delete;

        vk::Framebuffer getFramebuffer(int index) { return swapchain_framebuffers[index]; }
        vk::RenderPass getRenderPass() { return render_pass; }
        vk::ImageView getImageView(int index) { return swapchain_image_views[index]; }
        size_t getImageCount() { return swapchain_images.size(); }
        vk::Format getSwapchainImageFormat() { return swapchain_image_format; }
        vk::Extent2D getSwapchainExtent() { return swapchain_extent; }
        uint32_t getWidth() { return swapchain_extent.width; }
        uint32_t getHeight() { return swapchain_extent.height; }
        float extentAspectRatio() { return static_cast<float>(swapchain_extent.width) / static_cast<float>(swapchain_extent.height); }

        vk::Format findDepthFormat();
        vk::Result acquireNextImage(uint32_t *image_index);
        vk::Result submitCommandBuffers(const vk::CommandBuffer *buffers, uint32_t *image_index);
        bool compareSwapFormats(const Swapchain &swapchain) const;

    private:
        vk::Format swapchain_image_format;
        vk::Format swapchain_depth_format;
        vk::Extent2D swapchain_extent;

        std::vector<vk::Framebuffer> swapchain_framebuffers;
        vk::RenderPass render_pass;

        std::vector<vk::Image> depth_images;
        std::vector<vk::DeviceMemory> depth_image_memories;
        std::vector<vk::ImageView> depth_image_views;
        std::vector<vk::Image> swapchain_images;
        std::vector<vk::ImageView> swapchain_image_views;

        Device &device;
        vk::Extent2D window_extent;

        vk::SwapchainKHR swapchain;
        std::shared_ptr<Swapchain> old_swapchain;

        std::vector<vk::Semaphore> image_available_semaphores;
        std::vector<vk::Semaphore> render_finished_semaphores;
        std::vector<vk::Fence> in_flight_fences;
        std::vector<vk::Fence> images_in_flight;
        size_t current_frame = 0;

        void init();
        void createSwapchain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();

        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &available_formats);
        vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &available_present_modes);
        vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);
    };

}
