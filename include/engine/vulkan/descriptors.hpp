#pragma once

#include "engine/vulkan/device.hpp"

namespace muon {

    class DescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(Device &device) : device{device} {}

            Builder &addBinding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags, uint32_t count = 1);
            std::unique_ptr<DescriptorSetLayout> build() const;

        private:
            Device &device;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        DescriptorSetLayout(Device &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~DescriptorSetLayout();

        DescriptorSetLayout(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptor_set_layout; }

    private:
        Device &device;
        VkDescriptorSetLayout descriptor_set_layout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class DescriptorWriter;
    };

    class DescriptorPool {
    public:
        class Builder {
        public:
            Builder(Device &device) : device{device} {}

            Builder &addPoolSize(VkDescriptorType descriptor_type, uint32_t count);
            Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder &setMaxSets(uint32_t count);
            std::unique_ptr<DescriptorPool> build() const;

        private:
            Device &device;
            std::vector<VkDescriptorPoolSize> pool_sizes{};
            uint32_t max_sets = 1000;
            VkDescriptorPoolCreateFlags pool_flags = 0;
        };

        DescriptorPool(Device &device, uint32_t max_sets, VkDescriptorPoolCreateFlags pool_flags, const std::vector<VkDescriptorPoolSize> &pool_sizes);
        ~DescriptorPool();

        DescriptorPool(const DescriptorPool &) = delete;
        DescriptorPool &operator=(const DescriptorPool &) = delete;

        bool allocateDescriptor(const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet &descriptor) const;
        void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;
        void resetPool();

    private:
        Device &device;
        VkDescriptorPool descriptor_pool;

        friend class DescriptorWriter;
    };

    class DescriptorWriter {
    public:
        DescriptorWriter(DescriptorSetLayout &set_layout, DescriptorPool &pool);

        DescriptorWriter &writeToBuffer(uint32_t binding, VkDescriptorBufferInfo *buffer_info);
        DescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *image_info);

        bool build(VkDescriptorSet &set);
        void overwrite(VkDescriptorSet &set);

    private:
        DescriptorSetLayout &set_layout;
        DescriptorPool &pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}
