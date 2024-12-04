#include "engine/vulkan/texture.hpp"
#include "engine/assets/imageloader.hpp"

#include <cmath>
#include <vulkan/vulkan.hpp>

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

        image_format = vk::Format::eR8G8B8A8Srgb;
        instance_size = 4;

        createTexture(image_data.data());
    }

    Texture::Texture(Device &device, TextureCreateInfo &info) : device{device}, width{info.width}, height{info.height},
    image_format{info.image_format}, instance_size{info.instance_size} {
        createTexture(info.image_data);
    }

    Texture::~Texture() {
        device.getDevice().destroyImage(image, nullptr);
        device.getDevice().freeMemory(image_memory, nullptr);
        device.getDevice().destroyImageView(image_view, nullptr);
        device.getDevice().destroySampler(sampler, nullptr);
    }

    vk::DescriptorImageInfo Texture::descriptorInfo() const {
        vk::DescriptorImageInfo image_info{};
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
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        };

        staging_buffer.map();
        staging_buffer.writeToBuffer(image_data);

        vk::ImageCreateInfo image_info{};
        image_info.sType = vk::StructureType::eImageCreateInfo;
        image_info.imageType = vk::ImageType::e2D;
        image_info.extent.width = width;
        image_info.extent.height = height;
        image_info.extent.depth = 1;
        image_info.format = image_format;
        image_info.mipLevels = 1;
        image_info.arrayLayers = 1;
        image_info.samples = vk::SampleCountFlagBits::e1;
        image_info.tiling = vk::ImageTiling::eOptimal;
        image_info.initialLayout = vk::ImageLayout::eUndefined;
        image_info.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
        image_info.sharingMode = vk::SharingMode::eExclusive;

        device.createImageWithInfo(image_info, vk::MemoryPropertyFlagBits::eDeviceLocal, image, image_memory);

        transitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

        device.copyBufferToImage(staging_buffer.getBuffer(), image, width, height, 1);

        transitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        image_layout = vk::ImageLayout::eShaderReadOnlyOptimal;

        vk::SamplerCreateInfo sampler_info{};
        sampler_info.sType = vk::StructureType::eSamplerCreateInfo;
        sampler_info.minFilter = vk::Filter::eLinear;
        sampler_info.magFilter = vk::Filter::eLinear;
        sampler_info.mipmapMode = vk::SamplerMipmapMode::eLinear;
        sampler_info.addressModeU = vk::SamplerAddressMode::eRepeat;
        sampler_info.addressModeV = vk::SamplerAddressMode::eRepeat;
        sampler_info.addressModeW = vk::SamplerAddressMode::eRepeat;
        sampler_info.mipLodBias = 0.0f;
        sampler_info.compareOp = vk::CompareOp::eNever;
        sampler_info.minLod = 0.0f;
        sampler_info.maxLod = 0.0f;
        sampler_info.maxAnisotropy = 4.0f;
        sampler_info.anisotropyEnable = VK_TRUE;
        sampler_info.borderColor = vk::BorderColor::eFloatOpaqueWhite;

        device.getDevice().createSampler(&sampler_info, nullptr, &sampler);

        vk::ImageViewCreateInfo image_view_info{};
        image_view_info.sType = vk::StructureType::eImageViewCreateInfo;
        image_view_info.image = image;
        image_view_info.viewType = vk::ImageViewType::e2D;
        image_view_info.format = image_format;
        image_view_info.components = { vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };
        image_view_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        image_view_info.subresourceRange.baseMipLevel = 0;
        image_view_info.subresourceRange.levelCount = 1;
        image_view_info.subresourceRange.baseArrayLayer = 0;
        image_view_info.subresourceRange.layerCount = 1;

        device.getDevice().createImageView(&image_view_info, nullptr, &image_view);
    }

    void Texture::transitionImageLayout(vk::ImageLayout old_layout, vk::ImageLayout new_layout) {
        vk::CommandBuffer command_buffer = device.beginSingleTimeCommands();

        vk::ImageMemoryBarrier barrier{};
        barrier.sType = vk::StructureType::eImageMemoryBarrier;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vk::PipelineStageFlags source_stage;
        vk::PipelineStageFlags destination_stage;

        if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal) {
            barrier.srcAccessMask = vk::AccessFlags{};
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
            destination_stage = vk::PipelineStageFlagBits::eTransfer;
        } else if (old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            source_stage = vk::PipelineStageFlagBits::eTransfer;
            destination_stage = vk::PipelineStageFlagBits::eFragmentShader;
        } else {
            spdlog::error("Unsupported layout transition, exiting");
            exit(exitcode::FAILURE);
        }

        command_buffer.pipelineBarrier(source_stage, destination_stage, vk::DependencyFlags{}, 0, nullptr, 0, nullptr, 1, &barrier);

        device.endSingleTimeCommands(command_buffer);
    }

}
