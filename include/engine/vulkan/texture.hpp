#pragma once

#include <string>

#include <vulkan/vulkan.hpp>

#include "engine/vulkan/device.hpp"

namespace muon {

    struct TextureCreateInfo {
        vk::Format image_format;
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

        vk::Sampler getSampler() const { return sampler; }
        vk::ImageView getImageView() const { return image_view; }
        vk::ImageLayout getImageLayout() const { return image_layout; }

        vk::DescriptorImageInfo descriptorInfo() const;

    private:
        Device &device;

        uint32_t width;
        uint32_t height;

        vk::Image image;
        vk::DeviceMemory image_memory;
        vk::Sampler sampler;
        vk::ImageView image_view;
        vk::ImageLayout image_layout;
        vk::Format image_format;
        uint32_t instance_size;

        void createTexture(void *image_data);

        void transitionImageLayout(vk::ImageLayout old_layout, vk::ImageLayout new_layout);
    };

}
