#pragma once

#include <string>

#include <vulkan/vulkan.hpp>

#include "engine/vulkan/device.hpp"

namespace muon {

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

        const uint32_t getWidth() const { return width; }
        const uint32_t getHeight() const { return height; }

        VkSampler getSampler() const { return sampler; }
        VkImageView getImageView() const { return image_view; }
        VkImageLayout getImageLayout() const { return image_layout; }

        VkDescriptorImageInfo descriptorInfo() const;

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

        void createTexture(void *image_data);

        void transitionImageLayout(VkImageLayout old_layout, VkImageLayout new_layout);
    };

}
