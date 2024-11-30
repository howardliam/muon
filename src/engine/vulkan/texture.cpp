#include "engine/vulkan/texture.hpp"
#include "engine/assets/imageloader.hpp"

#include <cmath>

// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>

#include "engine/vulkan/buffer.hpp"

namespace muon {

    Texture::Texture(Device &device, const std::string &path) : device{device} {
        // int w, h, channels;
        // stbi_uc *image_data = stbi_load(path.c_str(), &w, &h, &channels, STBI_rgb_alpha);

        // width = static_cast<uint32_t>(w);
        // height = static_cast<uint32_t>(h);

        // image_format = VK_FORMAT_R8G8B8A8_SRGB;
        // instance_size = 4;

        // createTexture(image_data);

        // stbi_image_free(image_data);

        PngProperties properties{};
        std::vector<uint8_t> image_data;
        readPngFile(path, image_data, properties);

        width = properties.width;
        height = properties.height;

        image_format = VK_FORMAT_R8G8B8A8_SRGB;
        instance_size = 4;

        createTexture(image_data.data());
    }

    Texture::Texture(Device &device, TextureCreateInfo &info) : device{device}, width{info.width}, height{info.height},
    image_format{info.image_format}, instance_size{info.instance_size} {
        createTexture(info.image_data);
    }

    Texture::~Texture() {
        vkDestroyImage(device.getDevice(), image, nullptr);
        vkFreeMemory(device.getDevice(), image_memory, nullptr);
        vkDestroyImageView(device.getDevice(), image_view, nullptr);
        vkDestroySampler(device.getDevice(), sampler, nullptr);
    }

    VkDescriptorImageInfo Texture::descriptorInfo() const {
        VkDescriptorImageInfo image_info{};
        image_info.sampler = sampler;
        image_info.imageView = image_view;
        image_info.imageLayout = image_layout;

        return image_info;
    }

    void Texture::createTexture(void *image_data) {
        Buffer staging_buffer{
            device,
            instance_size,
            width * height,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        staging_buffer.map();
        staging_buffer.writeToBuffer(image_data);

        VkImageCreateInfo image_info{};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.extent.width = width;
        image_info.extent.height = height;
        image_info.extent.depth = 1;
        image_info.format = image_format;
        image_info.mipLevels = 1;
        image_info.arrayLayers = 1;
        image_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        device.createImageWithInfo(image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, image_memory);

        transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        device.copyBufferToImage(staging_buffer.getBuffer(), image, width, height, 1);

        transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkSamplerCreateInfo sampler_info{};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.minFilter = VK_FILTER_LINEAR;
        sampler_info.magFilter = VK_FILTER_LINEAR;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.mipLodBias = 0.0f;
        sampler_info.compareOp = VK_COMPARE_OP_NEVER;
        sampler_info.minLod = 0.0f;
        sampler_info.maxLod = 0.0f;
        sampler_info.maxAnisotropy = 4.0f;
        sampler_info.anisotropyEnable = VK_TRUE;
        sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        vkCreateSampler(device.getDevice(), &sampler_info, nullptr, &sampler);

        VkImageViewCreateInfo image_view_info{};
        image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_info.image = image;
        image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_info.format = image_format;
        image_view_info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_info.subresourceRange.baseMipLevel = 0;
        image_view_info.subresourceRange.levelCount = 1;
        image_view_info.subresourceRange.baseArrayLayer = 0;
        image_view_info.subresourceRange.layerCount = 1;

        vkCreateImageView(device.getDevice(), &image_view_info, nullptr, &image_view);
    }

    void Texture::transitionImageLayout(VkImageLayout old_layout, VkImageLayout new_layout) {
        VkCommandBuffer command_buffer = device.beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags destination_stage;

        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            spdlog::error("Unsupported layout transition, exiting");
            exit(exitcode::FAILURE);
        }

        vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        device.endSingleTimeCommands(command_buffer);
    }

}
