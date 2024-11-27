#pragma once

#include <vulkan/vulkan.hpp>
#include <spdlog/spdlog.h>

#include "engine/window/window.hpp"

namespace muon {

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
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

        VkInstance getInstance() const { return instance; }
        VkPhysicalDevice getPhysicalDevice() const { return physical_device; }
        VkCommandPool getCommandPool() const { return command_pool; }
        VkDevice getDevice() const { return device; }
        VkSurfaceKHR getSurface() const { return surface; }
        VkQueue getGraphicsQueue() const { return graphics_queue; }
        VkQueue getPresentQueue() const { return present_queue; }
        SwapChainSupportDetails getSwapchainSupport() { return querySwapchainSupport(physical_device); }
        QueueFamilyIndices getPhysicalQueueFamilies() { return findQueueFamilies(physical_device); }

        uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);
        VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory);
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer command_buffer);
        void copyBuffer(VkBuffer src_buffer, VkBuffer dest_buffer, VkDeviceSize size);
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer_count);
        void createImageWithInfo(const VkImageCreateInfo &image_info, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory);

    private:
        VkInstance instance{};
        VkDebugUtilsMessengerEXT debug_messenger{};
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        Window &window;
        VkCommandPool command_pool{};

        VkDevice device{};
        VkSurfaceKHR surface{};
        VkQueue graphics_queue{};
        VkQueue present_queue{};

        VkPhysicalDeviceProperties properties{};

        const std::vector<const char *> validation_layers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        bool isDeviceSuitable(VkPhysicalDevice device);
        std::vector<const char *> getRequiredExtensions();
        bool checkValidationLayerSupport();
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &create_info);
        void hasSdlRequiredInstanceExtensions();
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        SwapChainSupportDetails querySwapchainSupport(VkPhysicalDevice device);
    };

}
