#include "engine/vulkan/shaderreflection.hpp"

#include <algorithm>

#include <spdlog/spdlog.h>

#include "utils/exitcode.hpp"

namespace muon {

    uint32_t formatByteSize(vk::Format format) {
        uint32_t byte_size;

        switch (format) {
            case vk::Format::eR32G32Sfloat:
                byte_size = 2 * sizeof(float);
                break;

            case vk::Format::eR32G32B32Sfloat:
                byte_size = 3 * sizeof(float);
                break;

            default:
                spdlog::error("New format provided");
                break;
        }

        return byte_size;
    }

    ShaderReflection::ShaderReflection(std::vector<char> &data) {
        SpvReflectResult result = spvReflectCreateShaderModule(data.size(), data.data(), &module);
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            spvReflectDestroyShaderModule(&module);
            spdlog::error("Failed to create reflect shader module, exiting");
            exit(exitcode::FAILURE);
        }

        computeVertexInfo();
        computeDescriptorSetLayout();
    }

    ShaderReflection::~ShaderReflection() {
        spvReflectDestroyShaderModule(&module);
    }

    void ShaderReflection::computeVertexInfo() {
        uint32_t var_count = 0;
        auto result = spvReflectEnumerateInputVariables(&module, &var_count, nullptr);
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            spdlog::warn("Failed to enumerate input variables, returning early");
            return;
        }

        std::vector<SpvReflectInterfaceVariable *> input_vars(var_count);
        result = spvReflectEnumerateInputVariables(&module, &var_count, input_vars.data());
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            spdlog::warn("Failed to enumerate input variables, returning early");
            return;
        }

        std::sort(input_vars.begin(), input_vars.end(),
            [](const SpvReflectInterfaceVariable *a, const SpvReflectInterfaceVariable *b) {
                return a->location < b->location;
            }
        );

        vertex_info.attribute_descriptions.reserve(var_count);

        uint32_t total_offset = 0;
        for (int i = 0; i < var_count; i++) {
            const SpvReflectInterfaceVariable &var = *(input_vars[i]);

            auto format = vk::Format(var.format);
            vertex_info.attribute_descriptions.push_back({
                var.location,
                0,
                format,
                total_offset
            });
            auto offset = formatByteSize(vk::Format(var.format));
            total_offset += offset;
        }

        vertex_info.binding_descriptions.push_back({
            0,
            total_offset,
            vk::VertexInputRate::eVertex
        });
    }


    void ShaderReflection::computeDescriptorSetLayout() {
        uint32_t desc_count = 0;
        auto result = spvReflectEnumerateDescriptorSets(&module, &desc_count, nullptr);
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            spdlog::warn("Failed to enumerate descriptor sets, returning early");
            return;
        }

        std::vector<SpvReflectDescriptorSet *> desc_sets(desc_count);
        result = spvReflectEnumerateDescriptorSets(&module, &desc_count, desc_sets.data());
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            spdlog::warn("Failed to enumerate descriptor sets, returning early");
            return;
        }

        for (int i = 0; i < desc_count; i++) {
            spdlog::debug("Set location: {}", desc_sets[i]->set);

            for (int j = 0; j < desc_sets[i]->binding_count; j++) {
                spdlog::debug("Binding location: {}", j);
                spdlog::debug("Descriptor name: {}", desc_sets[i]->bindings[j]->name);
            }
        }
    }
}
