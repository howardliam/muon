#pragma once

#include <vulkan/vulkan.hpp>

#include "device.hpp"

namespace muon {

    class Buffer {
    public:
        Buffer(Device &device, vk::DeviceSize instance_size, uint32_t instance_count,
            vk::BufferUsageFlags usage_flags, vk::MemoryPropertyFlags memory_property_flags, vk::DeviceSize min_offset_alignment = 1);
        ~Buffer();

        Buffer(const Buffer &) = delete;
        Buffer& operator=(const Buffer &) = delete;

        vk::Result map(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);
        void unmap();

        void writeToBuffer(void *data, vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);
        vk::Result flush(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);
        vk::DescriptorBufferInfo descriptorInfo(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);
        vk::Result invalidate(vk::DeviceSize size = vk::WholeSize, vk::DeviceSize offset = 0);

        void writeToIndex(void* data, int32_t index);
        vk::Result flushIndex(int32_t index);
        vk::DescriptorBufferInfo descriptorInfoForIndex(int32_t index);
        vk::Result invalidateIndex(int32_t index);

        vk::Buffer getBuffer() const { return buffer; }
        vk::DeviceSize getBufferSize() const { return buffer_size; }
        void *getMappedMemory() const { return mapped; }
        uint32_t getInstanceCount() const { return instance_count; }
        vk::DeviceSize getInstanceSize() const { return instance_size; }
        vk::DeviceSize getAlignmentSize() const { return alignment_size; }
        vk::BufferUsageFlags getUsageFlags() const { return usage_flags; }
        vk::MemoryPropertyFlags getMemoryPropertyFlags() const { return memory_property_flags; }

    private:
        Device &device;
        vk::DeviceMemory memory = nullptr;

        vk::Buffer buffer = nullptr;
        vk::DeviceSize buffer_size;
        void *mapped = nullptr;
        uint32_t instance_count;
        vk::DeviceSize instance_size;
        vk::DeviceSize alignment_size;
        vk::BufferUsageFlags usage_flags;
        vk::MemoryPropertyFlags memory_property_flags;
    };

    vk::DeviceSize getAlignment(vk::DeviceSize instance_size, vk::DeviceSize min_offset_alignment);

}
