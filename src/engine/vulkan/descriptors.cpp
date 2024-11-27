#include "engine/vulkan/descriptors.hpp"

#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

namespace muon {

    /* DescriptorSetLayout Builder */
    DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags, uint32_t count) {
        VkDescriptorSetLayoutBinding layout_binding{};
        layout_binding.binding = binding;
        layout_binding.descriptorType = descriptor_type;
        layout_binding.descriptorCount = count;
        layout_binding.stageFlags = stage_flags;

        bindings[binding] = layout_binding;

        return *this;
    }

    std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
        return std::make_unique<DescriptorSetLayout>(device, bindings);
    }

    /* DescriptorSetLayout */
    DescriptorSetLayout::DescriptorSetLayout(Device &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) : device{device}, bindings{bindings} {
        std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings{};
        for (auto kv : bindings) {
            set_layout_bindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info{};
        descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_info.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
        descriptor_set_layout_info.pBindings = set_layout_bindings.data();

        if (vkCreateDescriptorSetLayout(device.getDevice(), &descriptor_set_layout_info, nullptr, &descriptor_set_layout) != VK_SUCCESS) {
            spdlog::error("Failed to create descriptor set layout");
            exit(exitcode::FAILURE);
        }
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(device.getDevice(), descriptor_set_layout, nullptr);
    }

    /* DescriptorPool Builder */
    DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptor_type, uint32_t count) {
        pool_sizes.push_back({descriptor_type, count});
        return *this;
    }

    DescriptorPool::Builder &DescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags) {
        pool_flags = flags;
        return *this;
    }

    DescriptorPool::Builder &DescriptorPool::Builder::setMaxSets(uint32_t count) {
        max_sets = count;
        return *this;
    }

    std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
        return std::make_unique<DescriptorPool>(device, max_sets, pool_flags, pool_sizes);
    }

    /* DescriptorPool */
    DescriptorPool::DescriptorPool(Device &device, uint32_t max_sets, VkDescriptorPoolCreateFlags pool_flags,
        const std::vector<VkDescriptorPoolSize> &pool_sizes) : device{device} {
        VkDescriptorPoolCreateInfo descriptor_pool_info{};
        descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        descriptor_pool_info.pPoolSizes = pool_sizes.data();
        descriptor_pool_info.maxSets = max_sets;
        descriptor_pool_info.flags = pool_flags;

        if (vkCreateDescriptorPool(device.getDevice(), &descriptor_pool_info, nullptr, &descriptor_pool) != VK_SUCCESS) {
            spdlog::error("Failed to create descriptor pool");
            exit(exitcode::FAILURE);
        }
    }

    DescriptorPool::~DescriptorPool() {
        vkDestroyDescriptorPool(device.getDevice(), descriptor_pool, nullptr);
    }

    bool DescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet &descriptor) const {
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = descriptor_pool;
        alloc_info.pSetLayouts = &descriptor_set_layout;
        alloc_info.descriptorSetCount = 1;

        if (vkAllocateDescriptorSets(device.getDevice(), &alloc_info, &descriptor) != VK_SUCCESS) {
            return false;
        }
        return true;
    }

    void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
        vkFreeDescriptorSets(device.getDevice(), descriptor_pool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
    }

    void DescriptorPool::resetPool() {
        vkResetDescriptorPool(device.getDevice(), descriptor_pool, 0);
    }

    /* DescriptorWriter */
    DescriptorWriter::DescriptorWriter(DescriptorSetLayout &set_layout, DescriptorPool &pool) : set_layout{set_layout}, pool{pool} {}

    DescriptorWriter &DescriptorWriter::writeToBuffer(uint32_t binding, VkDescriptorBufferInfo *buffer_info) {
        auto &binding_description = set_layout.bindings[binding];

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = binding_description.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = buffer_info;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    DescriptorWriter &DescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo *image_info) {
        auto &binding_description = set_layout.bindings[binding];

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = binding_description.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = image_info;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    bool DescriptorWriter::build(VkDescriptorSet &set) {
        bool success = pool.allocateDescriptor(set_layout.getDescriptorSetLayout(), set);
        if (!success) {
            return false;
        }
        overwrite(set);
        return true;
    }

    void DescriptorWriter::overwrite(VkDescriptorSet &set) {
        for (auto &write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.device.getDevice(), writes.size(), writes.data(), 0, nullptr);
    }

}
