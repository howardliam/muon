#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.hpp>
#include <spirv_reflect.h>

namespace muon {

    class ShaderReflection {
    public:
        struct VertexInfo {
            std::vector<vk::VertexInputAttributeDescription> attribute_descriptions{};
            std::vector<vk::VertexInputBindingDescription> binding_descriptions{};
        };

        ShaderReflection(std::vector<char> &data);
        ~ShaderReflection();

        void computeVertexInfo();
        void computePushConstant();
        void computeDescriptorSetLayout();

        VertexInfo getVertexInfo() { return vertex_info; }

    private:
        SpvReflectShaderModule module;

        VertexInfo vertex_info;
    };

}
