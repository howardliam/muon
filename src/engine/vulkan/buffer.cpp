#include "engine/vulkan/buffer.hpp"

namespace muon {

    Buffer::Buffer(Device &device, vk::DeviceSize instance_size, uint32_t instance_count,
        vk::BufferUsageFlags usage_flags, vk::MemoryPropertyFlags memory_property_flags, vk::DeviceSize min_offset_alignment)
        : device{device}, instance_size{instance_size}, instance_count{instance_count}, usage_flags{usage_flags}, memory_property_flags{memory_property_flags} {

        alignment_size = getAlignment(instance_size, min_offset_alignment);
        buffer_size = alignment_size * instance_count;
        device.createBuffer(buffer_size, usage_flags, memory_property_flags, buffer, memory);
    }

    Buffer::~Buffer() {
        unmap();
        device.getDevice().destroyBuffer(buffer, nullptr);
        device.getDevice().freeMemory(memory, nullptr);
    }

    vk::Result Buffer::map(vk::DeviceSize size, vk::DeviceSize offset) {
        return device.getDevice().mapMemory(memory, offset, size, vk::MemoryMapFlags{}, &mapped);
    }

    void Buffer::unmap() {
        if (mapped) {
            device.getDevice().unmapMemory(memory);
            mapped = nullptr;
        }
    }


    void Buffer::writeToBuffer(void *data, vk::DeviceSize size, vk::DeviceSize offset) {
        if (size == vk::WholeSize) {
            memcpy(mapped, data, buffer_size);
        } else {
            auto memory_offset = static_cast<char *>(mapped);
            memory_offset += offset;
            memcpy(memory_offset, data, size);
        }
    }

    vk::Result Buffer::flush(vk::DeviceSize size, vk::DeviceSize offset) {
        vk::MappedMemoryRange mapped_range = {};
        mapped_range.sType = vk::StructureType::eMappedMemoryRange;
        mapped_range.memory = memory;
        mapped_range.offset = offset;
        mapped_range.size = size;
        return device.getDevice().flushMappedMemoryRanges(1, &mapped_range);
    }

    vk::DescriptorBufferInfo Buffer::descriptorInfo(vk::DeviceSize size, vk::DeviceSize offset) {
        return vk::DescriptorBufferInfo{ buffer, offset, size };
    }

    vk::Result Buffer::invalidate(vk::DeviceSize size, vk::DeviceSize offset) {
        vk::MappedMemoryRange mapped_range = {};
        mapped_range.sType = vk::StructureType::eMappedMemoryRange;
        mapped_range.memory = memory;
        mapped_range.offset = offset;
        mapped_range.size = size;
        return device.getDevice().invalidateMappedMemoryRanges(1, &mapped_range);
    }


    void Buffer::writeToIndex(void* data, int index) {
        writeToBuffer(data, instance_size, index * alignment_size);
    }

    vk::Result Buffer::flushIndex(int index) {
        return flush(alignment_size, index * alignment_size);
    }

    vk::DescriptorBufferInfo Buffer::descriptorInfoForIndex(int index) {
        return descriptorInfo(alignment_size, index * alignment_size);
    }

    vk::Result Buffer::invalidateIndex(int index) {
        return invalidate(alignment_size, index * alignment_size);
    }

    vk::DeviceSize getAlignment(vk::DeviceSize instance_size, vk::DeviceSize min_offset_alignment) {
        if (min_offset_alignment > 0) {
            return (instance_size + min_offset_alignment - 1) & ~(min_offset_alignment - 1);
        }
        return instance_size;
    }

}
