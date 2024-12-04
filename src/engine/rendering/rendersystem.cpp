#include "engine/rendering/rendersystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace muon {

    glm::mat4 transformData() {
        glm::mat4 transform;

        transform = glm::translate(glm::mat4{1.0f}, {0.0f, 0.0f, -5.0f});
        float scale_factor = 0.1f;
        transform = glm::scale(transform, {scale_factor, scale_factor, scale_factor});

        return transform;
    }

    glm::mat4 transform = transformData();

    struct SimplePushConstantData {
        glm::mat4 model{1.0f};
    };

    RenderSystem3D::RenderSystem3D(Device &device, vk::RenderPass render_pass, vk::DescriptorSetLayout descriptor_set_layout) : device{device} {
        createPipelineLayout(descriptor_set_layout);
        createPipeline(render_pass);
    }

    RenderSystem3D::~RenderSystem3D() {
        vkDestroyPipelineLayout(device.getDevice(), pipeline_layout, nullptr);
    }

    void RenderSystem3D::renderModel(FrameInfo &frame_info, Model &model) {
        pipeline->bind(frame_info.command_buffer);

        frame_info.command_buffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            pipeline_layout,
            0,
            1,
            &frame_info.descriptor_set,
            0,
            nullptr
        );

        // transform = glm::rotate(transform, glm::radians(1.0f), {0.0f, 1.0f, 0.0f});

        SimplePushConstantData push{};
        push.model = transform;

        auto shader_stages = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
        frame_info.command_buffer.pushConstants(pipeline_layout, shader_stages, 0, sizeof(SimplePushConstantData), &push);

        model.bind(frame_info.command_buffer);
        model.draw(frame_info.command_buffer);
    }

    void RenderSystem3D::createPipelineLayout(vk::DescriptorSetLayout descriptor_set_layout) {
        vk::PushConstantRange push_constant_range{};
        push_constant_range.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
        push_constant_range.offset = 0;
        push_constant_range.size = sizeof(SimplePushConstantData);

        std::vector<vk::DescriptorSetLayout> descriptor_set_layouts{descriptor_set_layout};

        vk::PipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType =  vk::StructureType::ePipelineLayoutCreateInfo;
        pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
        pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges = &push_constant_range;

        if (device.getDevice().createPipelineLayout(&pipeline_layout_info, nullptr, &pipeline_layout) != vk::Result::eSuccess) {
            spdlog::error("Failed to create pipeline layout");
            exit(exitcode::FAILURE);
        }
    }

    void RenderSystem3D::createPipeline(vk::RenderPass render_pass) {
        PipelineConfigInfo pipeline_config{};
        Pipeline::defaultPipelineConfigInfo(pipeline_config);
        pipeline_config.render_pass= render_pass;
        pipeline_config.pipeline_layout = pipeline_layout;

        pipeline = std::make_unique<Pipeline>(device, "assets/shaders/shader.vert.spv", "assets/shaders/text.frag.spv", pipeline_config);
    }

}
