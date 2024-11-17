#include "buffer.hpp"

Buffer::Buffer(Device &device, VkDeviceSize instance_size, uint32_t instance_count,
    VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_property_flags, VkDeviceSize min_offset_alignment)
    : device{device}, instance_size{instance_size}, instance_count{instance_count}, usage_flags{usage_flags}, memory_property_flags{memory_property_flags} {

    alignment_size = get_alignment(instance_size, min_offset_alignment);
    buffer_size = alignment_size * instance_count;
    device.create_buffer(buffer_size, usage_flags, memory_property_flags, buffer, memory);
}

Buffer::~Buffer() {
    unmap();
    vkDestroyBuffer(device.get_device(), buffer, nullptr);
    vkFreeMemory(device.get_device(), memory, nullptr);
}

VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
    return vkMapMemory(device.get_device(), memory, offset, size, 0, &mapped);
}

void Buffer::unmap() {
    if (mapped) {
        vkUnmapMemory(device.get_device(), memory);
        mapped = nullptr;
    }
}


void Buffer::write_to_buffer(void *data, VkDeviceSize size, VkDeviceSize offset) {
    if (size == VK_WHOLE_SIZE) {
        memcpy(mapped, data, buffer_size);
    } else {
        auto memOffset = static_cast<char*>(mapped);
        memOffset += offset;
        memcpy(memOffset, data, size);
    }
}

VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mapped_range = {};
    mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mapped_range.memory = memory;
    mapped_range.offset = offset;
    mapped_range.size = size;
    return vkFlushMappedMemoryRanges(device.get_device(), 1, &mapped_range);
}

VkDescriptorBufferInfo Buffer::descriptor_info(VkDeviceSize size, VkDeviceSize offset) {
    return VkDescriptorBufferInfo{
        buffer,
        offset,
        size,
    };
}

VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mapped_range = {};
    mapped_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mapped_range.memory = memory;
    mapped_range.offset = offset;
    mapped_range.size = size;
    return vkInvalidateMappedMemoryRanges(device.get_device(), 1, &mapped_range);
}


void Buffer::write_to_index(void* data, int index) {
    write_to_buffer(data, instance_size, index * alignment_size);
}

VkResult Buffer::flush_index(int index) {
    return flush(alignment_size, index * alignment_size);
}

VkDescriptorBufferInfo Buffer::descriptor_info_for_index(int index) {
    return descriptor_info(alignment_size, index * alignment_size);
}

VkResult Buffer::invalidate_index(int index) {
    return invalidate(alignment_size, index * alignment_size);
}

VkDeviceSize get_alignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment) {
    if (min_offset_alignment > 0) {
        return (instance_size + min_offset_alignment - 1) & ~(min_offset_alignment - 1);
    }
    return instance_size;
}
