#pragma once

#include <string>

#include <vulkan/vulkan.hpp>

#include "engine/vulkan/device.hpp"

struct TextureCreateInfo {
    VkFormat image_format;
    uint32_t instance_size;
    uint32_t width;
    uint32_t height;
    void *image_data;
};

class Texture {
public:
    Texture(Device &device, const std::string &path);
    Texture(Device &device, TextureCreateInfo &info);
    ~Texture();

    Texture(const Texture &) = delete;
    Texture& operator=(const Texture &) = delete;

    const uint32_t get_width() const { return width; }
    const uint32_t get_height() const { return height; }

    VkSampler get_sampler() const { return sampler; }
    VkImageView get_image_view() const { return image_view; }
    VkImageLayout get_image_layout() const { return image_layout; }

    VkDescriptorImageInfo descriptor_info() const;

private:
    Device &device;

    uint32_t width;
    uint32_t height;

    VkImage image;
    VkDeviceMemory image_memory;
    VkSampler sampler;
    VkImageView image_view;
    VkImageLayout image_layout;
    VkFormat image_format;
    uint32_t instance_size;

    void create_texture(void *image_data);

    void transition_image_layout(VkImageLayout old_layout, VkImageLayout new_layout);
};
