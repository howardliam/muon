#pragma once

#include <string>

#include <vulkan/vulkan.hpp>

#include "engine/vulkan/device.hpp"

namespace muon {

    struct PipelineConfigInfo {
        PipelineConfigInfo() = default;

        PipelineConfigInfo(const PipelineConfigInfo &) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo &) = delete;

        vk::PipelineViewportStateCreateInfo viewport_info;
        vk::PipelineInputAssemblyStateCreateInfo input_assembly_info;
        vk::PipelineRasterizationStateCreateInfo rasterization_info;
        vk::PipelineMultisampleStateCreateInfo multisample_info;
        vk::PipelineColorBlendAttachmentState colour_blend_attachment;
        vk::PipelineColorBlendStateCreateInfo colour_blend_info;
        vk::PipelineDepthStencilStateCreateInfo depth_stencil_info;
        std::vector<vk::DynamicState> dynamic_state_enables;
        vk::PipelineDynamicStateCreateInfo dynamic_state_info;
        vk::PipelineLayout pipeline_layout = nullptr;
        vk::RenderPass render_pass = nullptr;
        uint32_t subpass = 0;
    };

    class Pipeline {
    public:
        Pipeline(Device &device, const std::string &vert_path, const std::string &frag_path, const PipelineConfigInfo &config_info);
        ~Pipeline();

        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        void bind(vk::CommandBuffer command_buffer);

        static void defaultPipelineConfigInfo(PipelineConfigInfo &config_info);
    private:
        Device &device;
        vk::Pipeline graphics_pipeline;
        vk::ShaderModule vert_shader_module;
        vk::ShaderModule frag_shader_module;

        void createShaderModule(const std::vector<char> &byte_code, vk::ShaderModule *shader_module);
        void createGraphicsPipeline(const std::string &vert_path, const std::string &frag_path, const PipelineConfigInfo &config_info);
    };

    // static std::vector<char> read_file(const std::string &path);

}
