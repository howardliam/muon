#include "engine/vulkan/pipeline.hpp"

#include <cstdlib>
#include <fstream>

#include "engine/vulkan/model.hpp"

#include <spirv_reflect.h>
#include <vulkan/vulkan.hpp>

namespace muon {

    std::vector<char> readFile(const std::string &path) {
        spdlog::debug("Loading shader: {}", path);
        std::ifstream file{path, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            spdlog::error("Failed to open file: {}", path);
            exit(exitcode::FAILURE);
        }

        size_t file_size = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(file_size);

        file.seekg(0);
        file.read(buffer.data(), file_size);

        SpvReflectShaderModule module;
        SpvReflectResult result = spvReflectCreateShaderModule(file_size, buffer.data(), &module);
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            spdlog::warn("Failed to create reflect shader module, returning early");
            spvReflectDestroyShaderModule(&module);
            return buffer;
        }

        uint32_t var_count = 0;
        result = spvReflectEnumerateInputVariables(&module, &var_count, nullptr);
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            spdlog::warn("Failed to enumerate input variables, returning early");
            spvReflectDestroyShaderModule(&module);
            return buffer;
        }

        std::vector<SpvReflectInterfaceVariable *> input_vars(var_count);
        result = spvReflectEnumerateInputVariables(&module, &var_count, input_vars.data());
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            spdlog::warn("Failed to enumerate input variables, returning early");
            spvReflectDestroyShaderModule(&module);
            return buffer;
        }

        for (int i = 0; i < var_count; i++) {
            spdlog::debug("Var loc: {}, var name: {}", input_vars[i]->location, input_vars[i]->name);
        }

        uint32_t desc_count = 0;
        result = spvReflectEnumerateDescriptorSets(&module, &desc_count, nullptr);
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            spdlog::warn("Failed to enumerate descriptor sets, returning early");
            spvReflectDestroyShaderModule(&module);
            return buffer;
        }

        std::vector<SpvReflectDescriptorSet *> desc_sets(desc_count);
        result = spvReflectEnumerateDescriptorSets(&module, &desc_count, desc_sets.data());
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            spdlog::warn("Failed to enumerate descriptor sets, returning early");
            spvReflectDestroyShaderModule(&module);
            return buffer;
        }

        for (int i = 0; i < desc_count; i++) {
            spdlog::debug("Set location: {}", desc_sets[i]->set);

            for (int j = 0; j < desc_sets[i]->binding_count; j++) {
                spdlog::debug("Binding location: {}", j);
                spdlog::debug("Descriptor name: {}", desc_sets[i]->bindings[j]->name);
            }
        }

        uint32_t push_constant_count = 0;
        result = spvReflectEnumeratePushConstantBlocks(&module, &push_constant_count, nullptr);
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            spdlog::warn("Failed to enumerate push constants, returning early");
            spvReflectDestroyShaderModule(&module);
            return buffer;
        }

        std::vector<SpvReflectBlockVariable *> push_constants(push_constant_count);
        result = spvReflectEnumeratePushConstantBlocks(&module, &push_constant_count, push_constants.data());
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            spdlog::warn("Failed to enumerate push constants, returning early");
            spvReflectDestroyShaderModule(&module);
            return buffer;
        }

        for (int i = 0; i < push_constant_count; i++) {
            spdlog::debug("Push constant: {}", push_constants[i]->name);
            spdlog::debug("Offset: {}", push_constants[i]->offset);
            spdlog::debug("Size: {}", push_constants[i]->size);

            for (int j = 0; j < push_constants[i]->member_count; j++) {
                spdlog::debug("Push constant var name: {}", push_constants[i]->members[j].name);
            }
        }

        spvReflectDestroyShaderModule(&module);

        return buffer;
    }

    Pipeline::Pipeline(Device &device, const std::string &vert_path, const std::string &frag_path, const PipelineConfigInfo &config_info) : device{device} {
        createGraphicsPipeline(vert_path, frag_path, config_info);
    }

    Pipeline::~Pipeline() {
        device.getDevice().destroyShaderModule(vert_shader_module, nullptr);
        device.getDevice().destroyShaderModule(frag_shader_module, nullptr);
        device.getDevice().destroyPipeline(graphics_pipeline, nullptr);
    }

    void Pipeline::bind(vk::CommandBuffer command_buffer) {
        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphics_pipeline);
    }

    void Pipeline::createShaderModule(const std::vector<char> &byte_code, vk::ShaderModule *shader_module) {
        vk::ShaderModuleCreateInfo create_info{};
        create_info.sType = vk::StructureType::eShaderModuleCreateInfo;
        create_info.codeSize = byte_code.size();
        create_info.pCode = reinterpret_cast<const uint32_t*>(byte_code.data());

        if (device.getDevice().createShaderModule(&create_info, nullptr, shader_module) != vk::Result::eSuccess) {
            spdlog::error("Failed to create shader module");
            exit(exitcode::FAILURE);
        }
    }

    void Pipeline::createGraphicsPipeline(const std::string &vert_path, const std::string &frag_path, const PipelineConfigInfo &config_info) {
        auto vert = readFile(vert_path);
        auto frag = readFile(frag_path);

        createShaderModule(vert, &vert_shader_module);
        createShaderModule(frag, &frag_shader_module);

        uint32_t num_stages = 2;
        vk::PipelineShaderStageCreateInfo shader_stages[num_stages];

        shader_stages[0].sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        shader_stages[0].stage = vk::ShaderStageFlagBits::eVertex;
        shader_stages[0].module = vert_shader_module;
        shader_stages[0].pName = "main";
        shader_stages[0].flags = vk::PipelineShaderStageCreateFlags{};
        shader_stages[0].pNext = nullptr;
        shader_stages[0].pSpecializationInfo = nullptr;

        shader_stages[1].sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        shader_stages[1].stage = vk::ShaderStageFlagBits::eFragment;
        shader_stages[1].module = frag_shader_module;
        shader_stages[1].pName = "main";
        shader_stages[1].flags = vk::PipelineShaderStageCreateFlags{};
        shader_stages[1].pNext = nullptr;
        shader_stages[1].pSpecializationInfo = nullptr;

        auto binding_descriptions = Model::Vertex::getBindingDescriptions();
        auto attribute_descriptions = Model::Vertex::getAttributeDescriptions();

        vk::PipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
        vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
        vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();
        vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();

        vk::GraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
        pipeline_info.stageCount = num_stages;
        pipeline_info.pStages = shader_stages;
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &config_info.input_assembly_info;
        pipeline_info.pViewportState = &config_info.viewport_info;
        pipeline_info.pRasterizationState = &config_info.rasterization_info;
        pipeline_info.pMultisampleState = &config_info.multisample_info;
        pipeline_info.pColorBlendState = &config_info.colour_blend_info;
        pipeline_info.pDepthStencilState = &config_info.depth_stencil_info;
        pipeline_info.pDynamicState = &config_info.dynamic_state_info;

        pipeline_info.layout = config_info.pipeline_layout;
        pipeline_info.renderPass = config_info.render_pass;
        pipeline_info.subpass = config_info.subpass;

        pipeline_info.basePipelineIndex = -1;
        pipeline_info.basePipelineHandle = nullptr;

        if (device.getDevice().createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline) != vk::Result::eSuccess) {
            spdlog::error("Failed to create graphics pipeline");
            exit(exitcode::FAILURE);
        }
    }

    void Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo &config_info) {
        config_info.input_assembly_info.sType = vk::StructureType::ePipelineInputAssemblyStateCreateInfo;
        config_info.input_assembly_info.topology = vk::PrimitiveTopology::eTriangleList;
        config_info.input_assembly_info.primitiveRestartEnable = vk::False;

        config_info.viewport_info.sType= vk::StructureType::ePipelineViewportStateCreateInfo;
        config_info.viewport_info.viewportCount= 1;
        config_info.viewport_info.pViewports = nullptr;
        config_info.viewport_info.scissorCount = 1;
        config_info.viewport_info.pScissors = nullptr;

        config_info.rasterization_info.sType = vk::StructureType::ePipelineRasterizationStateCreateInfo;
        config_info.rasterization_info.depthClampEnable = vk::False;
        config_info.rasterization_info.rasterizerDiscardEnable = vk::False;
        config_info.rasterization_info.polygonMode = vk::PolygonMode::eFill;
        config_info.rasterization_info.lineWidth = 1.0f;
        config_info.rasterization_info.cullMode = vk::CullModeFlagBits::eNone;
        config_info.rasterization_info.frontFace = vk::FrontFace::eClockwise;
        config_info.rasterization_info.depthBiasEnable = vk::False;
        config_info.rasterization_info.depthBiasConstantFactor = 0.0f;  // Optional
        config_info.rasterization_info.depthBiasClamp = 0.0f;           // Optional
        config_info.rasterization_info.depthBiasSlopeFactor = 0.0f;     // Optional

        config_info.multisample_info.sType = vk::StructureType::ePipelineMultisampleStateCreateInfo;
        config_info.multisample_info.sampleShadingEnable = vk::False;
        config_info.multisample_info.rasterizationSamples = vk::SampleCountFlagBits::e1;
        config_info.multisample_info.minSampleShading = 1.0f;           // Optional
        config_info.multisample_info.pSampleMask = nullptr;             // Optional
        config_info.multisample_info.alphaToCoverageEnable = vk::False;  // Optional
        config_info.multisample_info.alphaToOneEnable = vk::False;       // Optional

        config_info.colour_blend_attachment.colorWriteMask =
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA;
        config_info.colour_blend_attachment.blendEnable = vk::False;
        config_info.colour_blend_attachment.srcColorBlendFactor = vk::BlendFactor::eOne;   // Optional
        config_info.colour_blend_attachment.dstColorBlendFactor = vk::BlendFactor::eZero;  // Optional
        config_info.colour_blend_attachment.colorBlendOp = vk::BlendOp::eAdd;              // Optional
        config_info.colour_blend_attachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;   // Optional
        config_info.colour_blend_attachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;  // Optional
        config_info.colour_blend_attachment.alphaBlendOp = vk::BlendOp::eAdd;              // Optional

        config_info.colour_blend_info.sType = vk::StructureType::ePipelineColorBlendStateCreateInfo;
        config_info.colour_blend_info.logicOpEnable = vk::False;
        config_info.colour_blend_info.logicOp = vk::LogicOp::eCopy;  // Optional
        config_info.colour_blend_info.attachmentCount = 1;
        config_info.colour_blend_info.pAttachments = &config_info.colour_blend_attachment;
        config_info.colour_blend_info.setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f }); // Optional

        config_info.depth_stencil_info.sType = vk::StructureType::ePipelineDepthStencilStateCreateInfo;
        config_info.depth_stencil_info.depthTestEnable = vk::True;
        config_info.depth_stencil_info.depthWriteEnable = vk::True;
        config_info.depth_stencil_info.depthCompareOp = vk::CompareOp::eLess;
        config_info.depth_stencil_info.depthBoundsTestEnable = vk::False;
        config_info.depth_stencil_info.minDepthBounds = 0.0f;  // Optional
        config_info.depth_stencil_info.maxDepthBounds = 1.0f;  // Optional
        config_info.depth_stencil_info.stencilTestEnable = vk::False;
        config_info.depth_stencil_info.front = vk::StencilOpState{};  // Optional
        config_info.depth_stencil_info.back = vk::StencilOpState{};   // Optional

        config_info.dynamic_state_enables = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        config_info.dynamic_state_info.sType = vk::StructureType::ePipelineDynamicStateCreateInfo;
        config_info.dynamic_state_info.pDynamicStates = config_info.dynamic_state_enables.data();
        config_info.dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(config_info.dynamic_state_enables.size());
    }

}
