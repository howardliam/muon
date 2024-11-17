#pragma once

#include <string>

#include <vulkan/vulkan.hpp>

#include "device.hpp"

class Texture {
public:
    Texture(Device &device, const std::string &path);
    ~Texture();

    Texture(const Texture &) = delete;
    Texture& operator=(const Texture &) = delete;

    VkSampler get_sampler() const { return sampler; }
    VkImageView get_image_view() const { return image_view; }
    VkImageLayout get_image_layout() const { return image_layout; }

private:
    Device &device;

    VkImage image;
    VkDeviceMemory image_memory;
    VkSampler sampler;
    VkImageView image_view;
    VkImageLayout image_layout;
    VkFormat image_format;

    void transition_image_layout(VkImageLayout old_layout, VkImageLayout new_layout);
};
