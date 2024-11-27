#pragma once

#include <string>

#include <vulkan/vulkan.hpp>

#include "engine/vulkan/device.hpp"

namespace muon {

    struct PipelineConfigInfo {
        PipelineConfigInfo() = default;

        PipelineConfigInfo(const PipelineConfigInfo &) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo &) = delete;

        VkPipelineViewportStateCreateInfo viewport_info;
        VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
        VkPipelineRasterizationStateCreateInfo rasterisation_info;
        VkPipelineMultisampleStateCreateInfo multisample_info;
        VkPipelineColorBlendAttachmentState colour_blend_attachment;
        VkPipelineColorBlendStateCreateInfo colour_blend_info;
        VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
        std::vector<VkDynamicState> dynamic_state_enables;
        VkPipelineDynamicStateCreateInfo dynamic_state_info;
        VkPipelineLayout pipeline_layout = nullptr;
        VkRenderPass render_pass = nullptr;
        uint32_t subpass = 0;
    };

    class Pipeline {
    public:
        Pipeline(Device &device, const std::string &vert_path, const std::string &frag_path, const PipelineConfigInfo &config_info);
        ~Pipeline();

        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;

        void bind(VkCommandBuffer command_buffer);

        static void defaultPipelineConfigInfo(PipelineConfigInfo &config_info);
    private:
        Device &device;
        VkPipeline graphics_pipeline;
        VkShaderModule vert_shader_module;
        VkShaderModule frag_shader_module;


        void createShaderModule(const std::vector<char> &code, VkShaderModule *shader_module);
        void createGraphicsPipeline(const std::string &vert_path, const std::string &frag_path, const PipelineConfigInfo &config_info);
    };

    // static std::vector<char> read_file(const std::string &path);

}
