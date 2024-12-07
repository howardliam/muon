#include "engine/vulkan/pipeline.hpp"

#include <array>
#include <cstdlib>
#include <fstream>

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>
#include <spirv_reflect.h>

#include "engine/vulkan/model.hpp"
#include "engine/vulkan/shaderreflection.hpp"
#include "utils/exitcode.hpp"

namespace muon {

    std::vector<char> readFile(const std::string &path) {
        std::ifstream file{path, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            spdlog::error("Failed to open file: {}", path);
            exit(exitcode::FAILURE);
        }

        std::vector<char> buffer(file.tellg());

        file.seekg(0);
        file.read(buffer.data(), buffer.size());

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
        create_info.pCode = reinterpret_cast<const uint32_t *>(byte_code.data());

        if (device.getDevice().createShaderModule(&create_info, nullptr, shader_module) != vk::Result::eSuccess) {
            spdlog::error("Failed to create shader module");
            exit(exitcode::FAILURE);
        }
    }

    void Pipeline::createGraphicsPipeline(const std::string &vert_path, const std::string &frag_path, const PipelineConfigInfo &config_info) {
        auto vert = readFile(vert_path);
        auto frag = readFile(frag_path);

        ShaderReflection reflection{vert};
        auto vertex_info = reflection.getVertexInfo();

        createShaderModule(vert, &vert_shader_module);
        createShaderModule(frag, &frag_shader_module);

        auto a = offsetof(Model::Vertex, colour);

        std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages;

        size_t idx = 0;
        shader_stages[idx].sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        shader_stages[idx].stage = vk::ShaderStageFlagBits::eVertex;
        shader_stages[idx].module = vert_shader_module;
        shader_stages[idx].pName = "main";
        shader_stages[idx].flags = vk::PipelineShaderStageCreateFlags{};
        shader_stages[idx].pNext = nullptr;
        shader_stages[idx].pSpecializationInfo = nullptr;

        idx += 1;
        shader_stages[idx].sType = vk::StructureType::ePipelineShaderStageCreateInfo;
        shader_stages[idx].stage = vk::ShaderStageFlagBits::eFragment;
        shader_stages[idx].module = frag_shader_module;
        shader_stages[idx].pName = "main";
        shader_stages[idx].flags = vk::PipelineShaderStageCreateFlags{};
        shader_stages[idx].pNext = nullptr;
        shader_stages[idx].pSpecializationInfo = nullptr;

        vk::PipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = vk::StructureType::ePipelineVertexInputStateCreateInfo;
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_info.attribute_descriptions.size());
        vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_info.binding_descriptions.size());
        vertex_input_info.pVertexAttributeDescriptions = vertex_info.attribute_descriptions.data();
        vertex_input_info.pVertexBindingDescriptions = vertex_info.binding_descriptions.data();

        vk::GraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.sType = vk::StructureType::eGraphicsPipelineCreateInfo;
        pipeline_info.stageCount = static_cast<uint32_t>(shader_stages.size());
        pipeline_info.pStages = shader_stages.data();
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

        if (device.getDevice().createGraphicsPipelines(nullptr, 1, &pipeline_info, nullptr, &graphics_pipeline) != vk::Result::eSuccess) {
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
