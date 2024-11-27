#pragma once

#include <vulkan/vulkan.hpp>

#include "engine/vulkan/device.hpp"

namespace muon {

    class Swapchain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        Swapchain(Device &device, VkExtent2D window_extent);
        Swapchain(Device &device, VkExtent2D window_extent, std::shared_ptr<Swapchain> previous);
        ~Swapchain();

        Swapchain(const Swapchain &) = delete;
        Swapchain& operator=(const Swapchain &) = delete;

        VkFramebuffer getFramebuffer(int index) { return swapchain_framebuffers[index]; }
        VkRenderPass getRenderPass() { return render_pass; }
        VkImageView getImageView(int index) { return swapchain_image_views[index]; }
        size_t getImageCount() { return swapchain_images.size(); }
        VkFormat getSwapchainImageFormat() { return swapchain_image_format; }
        VkExtent2D getSwapchainExtent() { return swapchain_extent; }
        uint32_t getWidth() { return swapchain_extent.width; }
        uint32_t getHeight() { return swapchain_extent.height; }
        float extentAspectRatio() { return static_cast<float>(swapchain_extent.width) / static_cast<float>(swapchain_extent.height); }

        VkFormat findDepthFormat();
        VkResult acquireNextImage(uint32_t *image_index);
        VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *image_index);
        bool compareSwapFormats(const Swapchain &swapchain) const;

    private:
        VkFormat swapchain_image_format;
        VkFormat swapchain_depth_format;
        VkExtent2D swapchain_extent;

        std::vector<VkFramebuffer> swapchain_framebuffers;
        VkRenderPass render_pass;

        std::vector<VkImage> depth_images;
        std::vector<VkDeviceMemory> depth_image_memories;
        std::vector<VkImageView> depth_image_views;
        std::vector<VkImage> swapchain_images;
        std::vector<VkImageView> swapchain_image_views;

        Device &device;
        VkExtent2D window_extent;

        VkSwapchainKHR swapchain;
        std::shared_ptr<Swapchain> old_swapchain;

        std::vector<VkSemaphore> image_available_semaphores;
        std::vector<VkSemaphore> render_finished_semaphores;
        std::vector<VkFence> in_flight_fences;
        std::vector<VkFence> images_in_flight;
        size_t current_frame = 0;

        void init();
        void createSwapchain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &available_formats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &available_present_modes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    };

}
