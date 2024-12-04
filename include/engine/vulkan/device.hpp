#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "engine/window/window.hpp"

namespace muon {

    struct SwapchainSupportDetails {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> present_modes;
    };

    struct QueueFamilyIndices {
        uint32_t graphics_family{};
        uint32_t present_family{};
        bool graphics_family_has_value{false};
        bool present_family_has_value{false};

        bool isComplete() const {
            return graphics_family_has_value && present_family_has_value;
        }
    };

    #ifdef NDEBUG
        static constexpr bool enable_validation_layers = false;
    #else
        static constexpr bool enable_validation_layers = true;
    #endif

    class Device {
    public:
        Device(Window &window);
        ~Device();

        Device(const Device &) = delete;
        Device& operator=(const Device& ) = delete;
        Device(const Device &&) = delete;
        Device& operator=(const Device &&) = delete;

        vk::Instance getInstance() const { return instance; }
        vk::PhysicalDevice getPhysicalDevice() const { return physical_device; }
        vk::CommandPool getCommandPool() const { return command_pool; }
        vk::Device getDevice() const { return device; }
        vk::SurfaceKHR getSurface() const { return surface; }
        vk::Queue getGraphicsQueue() const { return graphics_queue; }
        vk::Queue getPresentQueue() const { return present_queue; }
        SwapchainSupportDetails getSwapchainSupport() { return querySwapchainSupport(physical_device); }
        QueueFamilyIndices getPhysicalQueueFamilies() { return findQueueFamilies(physical_device); }

        uint32_t findMemoryType(uint32_t type_filter, vk::MemoryPropertyFlags properties);
        vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

        void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &buffer_memory);
        vk::CommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(vk::CommandBuffer command_buffer);
        void copyBuffer(vk::Buffer src_buffer, vk::Buffer dest_buffer, vk::DeviceSize size);
        void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layer_count);
        void createImageWithInfo(const vk::ImageCreateInfo &image_info, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& image_memory);

    private:
        vk::Instance instance{};
        vk::DebugUtilsMessengerEXT debug_messenger{};
        vk::PhysicalDevice physical_device = nullptr;
        Window &window;
        vk::CommandPool command_pool{};

        vk::Device device{};
        vk::SurfaceKHR surface{};
        vk::Queue graphics_queue{};
        vk::Queue present_queue{};

        vk::PhysicalDeviceProperties properties{};

        const std::vector<const char *> validation_layers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        bool isDeviceSuitable(vk::PhysicalDevice device);
        std::vector<const char *> getRequiredExtensions();
        bool checkValidationLayerSupport();
        QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);
        void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT &create_info);
        void hasSdlRequiredInstanceExtensions();
        bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
        SwapchainSupportDetails querySwapchainSupport(vk::PhysicalDevice device);
    };

}
