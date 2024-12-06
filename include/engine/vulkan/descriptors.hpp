#pragma once

#include <unordered_map>
#include <memory>

#include <vulkan/vulkan.hpp>

#include "engine/vulkan/device.hpp"

namespace muon {

    class DescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(Device &device) : device{device} {}

            Builder &addBinding(uint32_t binding, vk::DescriptorType descriptor_type, vk::ShaderStageFlags stage_flags, uint32_t count = 1);
            std::unique_ptr<DescriptorSetLayout> build() const;

        private:
            Device &device;
            std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};
        };

        DescriptorSetLayout(Device &device, std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings);
        ~DescriptorSetLayout();

        DescriptorSetLayout(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

        vk::DescriptorSetLayout getDescriptorSetLayout() const { return descriptor_set_layout; }

    private:
        Device &device;
        vk::DescriptorSetLayout descriptor_set_layout;
        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings;

        friend class DescriptorWriter;
    };

    class DescriptorPool {
    public:
        class Builder {
        public:
            Builder(Device &device) : device{device} {}

            Builder &addPoolSize(vk::DescriptorType descriptor_type, uint32_t count);
            Builder &setPoolFlags(vk::DescriptorPoolCreateFlags flags);
            Builder &setMaxSets(uint32_t count);
            std::unique_ptr<DescriptorPool> build() const;

        private:
            Device &device;
            std::vector<vk::DescriptorPoolSize> pool_sizes{};
            uint32_t max_sets = 1000;
            vk::DescriptorPoolCreateFlags pool_flags = vk::DescriptorPoolCreateFlags{};
        };

        DescriptorPool(Device &device, uint32_t max_sets, vk::DescriptorPoolCreateFlags pool_flags, const std::vector<vk::DescriptorPoolSize> &pool_sizes);
        ~DescriptorPool();

        DescriptorPool(const DescriptorPool &) = delete;
        DescriptorPool &operator=(const DescriptorPool &) = delete;

        bool allocateDescriptor(const vk::DescriptorSetLayout descriptor_set_layout, vk::DescriptorSet &descriptor) const;
        void freeDescriptors(std::vector<vk::DescriptorSet> &descriptors) const;
        void resetPool();

    private:
        Device &device;
        vk::DescriptorPool descriptor_pool;

        friend class DescriptorWriter;
    };

    class DescriptorWriter {
    public:
        DescriptorWriter(DescriptorSetLayout &set_layout, DescriptorPool &pool);

        DescriptorWriter &writeToBuffer(uint32_t binding, vk::DescriptorBufferInfo *buffer_info);
        DescriptorWriter &writeImage(uint32_t binding, vk::DescriptorImageInfo *image_info);

        bool build(vk::DescriptorSet &set);
        void overwrite(vk::DescriptorSet &set);

    private:
        DescriptorSetLayout &set_layout;
        DescriptorPool &pool;
        std::vector<vk::WriteDescriptorSet> writes;
    };

}
