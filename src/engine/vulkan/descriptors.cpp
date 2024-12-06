#include "engine/vulkan/descriptors.hpp"

#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

#include "utils/exitcode.hpp"

namespace muon {

    /* DescriptorSetLayout Builder */
    DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::addBinding(uint32_t binding, vk::DescriptorType descriptor_type, vk::ShaderStageFlags stage_flags, uint32_t count) {
        vk::DescriptorSetLayoutBinding layout_binding{};
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
    DescriptorSetLayout::DescriptorSetLayout(Device &device, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings) : device{device}, bindings{bindings} {
        std::vector<vk::DescriptorSetLayoutBinding> set_layout_bindings{};
        for (auto [key, value] : bindings) {
            set_layout_bindings.push_back(value);
        }

        vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_info{};
        descriptor_set_layout_info.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
        descriptor_set_layout_info.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
        descriptor_set_layout_info.pBindings = set_layout_bindings.data();

        if (device.getDevice().createDescriptorSetLayout(&descriptor_set_layout_info, nullptr, &descriptor_set_layout) != vk::Result::eSuccess) {
            spdlog::error("Failed to create descriptor set layout");
            exit(exitcode::FAILURE);
        }
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(device.getDevice(), descriptor_set_layout, nullptr);
    }

    /* DescriptorPool Builder */
    DescriptorPool::Builder &DescriptorPool::Builder::addPoolSize(vk::DescriptorType descriptor_type, uint32_t count) {
        pool_sizes.push_back({descriptor_type, count});
        return *this;
    }

    DescriptorPool::Builder &DescriptorPool::Builder::setPoolFlags(vk::DescriptorPoolCreateFlags flags) {
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
    DescriptorPool::DescriptorPool(Device &device, uint32_t max_sets, vk::DescriptorPoolCreateFlags pool_flags,
        const std::vector<vk::DescriptorPoolSize> &pool_sizes) : device{device} {
        vk::DescriptorPoolCreateInfo descriptor_pool_info{};
        descriptor_pool_info.sType = vk::StructureType::eDescriptorPoolCreateInfo;
        descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
        descriptor_pool_info.pPoolSizes = pool_sizes.data();
        descriptor_pool_info.maxSets = max_sets;
        descriptor_pool_info.flags = pool_flags;

        if (device.getDevice().createDescriptorPool(&descriptor_pool_info, nullptr, &descriptor_pool) != vk::Result::eSuccess) {
            spdlog::error("Failed to create descriptor pool");
            exit(exitcode::FAILURE);
        }
    }

    DescriptorPool::~DescriptorPool() {
        vkDestroyDescriptorPool(device.getDevice(), descriptor_pool, nullptr);
    }

    bool DescriptorPool::allocateDescriptor(const vk::DescriptorSetLayout descriptor_set_layout, vk::DescriptorSet &descriptor) const {
        vk::DescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = vk::StructureType::eDescriptorSetAllocateInfo;
        alloc_info.descriptorPool = descriptor_pool;
        alloc_info.pSetLayouts = &descriptor_set_layout;
        alloc_info.descriptorSetCount = 1;

        if (device.getDevice().allocateDescriptorSets(&alloc_info, &descriptor) != vk::Result::eSuccess) {
            return false;
        }
        return true;
    }

    void DescriptorPool::freeDescriptors(std::vector<vk::DescriptorSet> &descriptors) const {
        device.getDevice().freeDescriptorSets(descriptor_pool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
    }

    void DescriptorPool::resetPool() {
        vkResetDescriptorPool(device.getDevice(), descriptor_pool, 0);
    }

    /* DescriptorWriter */
    DescriptorWriter::DescriptorWriter(DescriptorSetLayout &set_layout, DescriptorPool &pool) : set_layout{set_layout}, pool{pool} {}

    DescriptorWriter &DescriptorWriter::writeToBuffer(uint32_t binding, vk::DescriptorBufferInfo *buffer_info) {
        auto &binding_description = set_layout.bindings[binding];

        vk::WriteDescriptorSet write{};
        write.sType = vk::StructureType::eWriteDescriptorSet;
        write.descriptorType = binding_description.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = buffer_info;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    DescriptorWriter &DescriptorWriter::writeImage(uint32_t binding, vk::DescriptorImageInfo *image_info) {
        auto &binding_description = set_layout.bindings[binding];

        vk::WriteDescriptorSet write{};
        write.sType = vk::StructureType::eWriteDescriptorSet;
        write.descriptorType = binding_description.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = image_info;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    bool DescriptorWriter::build(vk::DescriptorSet &set) {
        bool success = pool.allocateDescriptor(set_layout.getDescriptorSetLayout(), set);
        if (!success) {
            return false;
        }
        overwrite(set);
        return true;
    }

    void DescriptorWriter::overwrite(vk::DescriptorSet &set) {
        for (auto &write : writes) {
            write.dstSet = set;
        }
        pool.device.getDevice().updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);
    }

}
