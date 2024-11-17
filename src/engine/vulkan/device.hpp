#pragma once

#include <vulkan/vulkan.hpp>
#include <spdlog/spdlog.h>

#include "../window/window.hpp"

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

    bool is_complete() const {
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

    VkInstance get_instance() const { return instance; }
    VkPhysicalDevice get_physical_device() const { return physical_device; }
    VkCommandPool get_command_pool() const { return command_pool; }
    VkDevice get_device() const { return device; }
    VkSurfaceKHR get_surface() const { return surface; }
    VkQueue get_graphics_queue() const { return graphics_queue; }
    VkQueue get_present_queue() const { return present_queue; }
    SwapChainSupportDetails get_swapchain_support() { return query_swapchain_support(physical_device); }
    QueueFamilyIndices get_physical_queue_families() { return find_queue_families(physical_device); }

    uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties);
    VkFormat find_supported_format(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory);
    VkCommandBuffer begin_single_time_commands();
    void end_single_time_commands(VkCommandBuffer command_buffer);
    void copy_buffer(VkBuffer src_buffer, VkBuffer dest_buffer, VkDeviceSize size);
    void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer_count);
    void create_image_with_info(const VkImageCreateInfo &image_info, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory);

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

    const std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    void create_instance();
    void setup_debug_messenger();
    void create_surface();
    void pick_physical_device();
    void create_logical_device();
    void create_command_pool();

    bool is_device_suitable(VkPhysicalDevice device);
    std::vector<const char*> get_required_extensions();
    bool check_validation_layer_support();
    QueueFamilyIndices find_queue_families(VkPhysicalDevice device);
    void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info);
    void has_sdl_required_instance_extensions();
    bool check_device_extension_support(VkPhysicalDevice device);
    SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice device);
};
