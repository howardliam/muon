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

        VkFramebuffer get_frame_buffer(int index) { return swapchain_framebuffers[index]; }
        VkRenderPass get_render_pass() { return render_pass; }
        VkImageView get_image_view(int index) { return swapchain_image_views[index]; }
        size_t get_image_count() { return swapchain_images.size(); }
        VkFormat get_swapchain_image_format() { return swapchain_image_format; }
        VkExtent2D get_swapchain_extent() { return swapchain_extent; }
        uint32_t get_width() { return swapchain_extent.width; }
        uint32_t get_height() { return swapchain_extent.height; }
        float extent_aspect_ratio() { return static_cast<float>(swapchain_extent.width) / static_cast<float>(swapchain_extent.height); }

        VkFormat find_depth_format();
        VkResult acquire_next_image(uint32_t *image_index);
        VkResult submit_command_buffers(const VkCommandBuffer *buffers, uint32_t *image_index);
        bool compare_swap_formats(const Swapchain &swapchain) const;

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
        void create_swapchain();
        void create_image_views();
        void create_depth_resources();
        void create_render_pass();
        void create_framebuffers();
        void create_sync_objects();

        VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats);
        VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes);
        VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities);
    };

}
