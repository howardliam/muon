#pragma once

#include <vulkan/vulkan.hpp>

#include "device.hpp"

namespace muon {

    class Buffer {
    public:
        Buffer(Device &device, VkDeviceSize instance_size, uint32_t instance_count,
            VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_property_flags, VkDeviceSize min_offset_alignment = 1);
        ~Buffer();

        Buffer(const Buffer &) = delete;
        Buffer& operator=(const Buffer &) = delete;

        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void unmap();

        void writeToBuffer(void *data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        void writeToIndex(void* data, int index);
        VkResult flushIndex(int index);
        VkDescriptorBufferInfo descriptorInfoForIndex(int index);
        VkResult invalidateIndex(int index);

        VkBuffer getBuffer() const { return buffer; }
        VkDeviceSize getBufferSize() const { return buffer_size; }
        void *getMappedMemory() const { return mapped; }
        uint32_t getInstanceCount() const { return instance_count; }
        VkDeviceSize getInstanceSize() const { return instance_size; }
        VkDeviceSize getAlignmentSize() const { return alignment_size; }
        VkBufferUsageFlags getUsageFlags() const { return usage_flags; }
        VkMemoryPropertyFlags getMemoryPropertyFlags() const { return memory_property_flags; }

    private:
        Device &device;
        VkDeviceMemory memory = VK_NULL_HANDLE;

        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceSize buffer_size;
        void *mapped = nullptr;
        uint32_t instance_count;
        VkDeviceSize instance_size;
        VkDeviceSize alignment_size;
        VkBufferUsageFlags usage_flags;
        VkMemoryPropertyFlags memory_property_flags;
    };

    VkDeviceSize getAlignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment);

}
