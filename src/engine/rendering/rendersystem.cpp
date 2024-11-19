#include "rendersystem.hpp"
#include <glm/trigonometric.hpp>
#include <vulkan/vulkan_core.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

struct SimplePushConstantData {
    glm::mat4 model{1.0f};
};

RenderSystem::RenderSystem(Device &device, VkRenderPass render_pass, VkDescriptorSetLayout descriptor_set_layout) : device{device} {
    create_pipeine_layout(descriptor_set_layout);
    create_pipeine(render_pass);
}

RenderSystem::~RenderSystem() {
    vkDestroyPipelineLayout(device.get_device(), pipeline_layout, nullptr);
}

void RenderSystem::render_model(Model &model, VkCommandBuffer command_buffer, VkDescriptorSet descriptor_set) {
    pipeline->bind(command_buffer);

    vkCmdBindDescriptorSets(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_layout,
        0,
        1,
        &descriptor_set,
        0,
        nullptr
    );

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.77778f, 0.01f, 1000.0f);
    glm::mat4 transform = glm::translate(glm::mat4{1.0f}, {0.0f, 0.0f, -10.0f});
    transform = glm::rotate(transform, glm::radians(30.0f), {1.0f, 1.0f, 1.0f});

    SimplePushConstantData push{};
    // push.model = projection * transform;
    push.model = transform;

    auto shader_stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    vkCmdPushConstants(command_buffer, pipeline_layout, shader_stages, 0, sizeof(SimplePushConstantData), &push);

    model.bind(command_buffer);
    model.draw(command_buffer);
}

void RenderSystem::create_pipeine_layout(VkDescriptorSetLayout descriptor_set_layout) {
    VkPushConstantRange push_constant_range{};
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(SimplePushConstantData);

    std::vector<VkDescriptorSetLayout> descriptor_set_layouts{descriptor_set_layout};

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType =  VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
    pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant_range;

    if (vkCreatePipelineLayout(device.get_device(), &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
        spdlog::error("Failed to create pipeline layout");
        exit(exitcode::FAILURE);
    }
}

void RenderSystem::create_pipeine(VkRenderPass render_pass) {
    PipelineConfigInfo pipeline_config{};
    Pipeline::default_pipeline_config_info(pipeline_config);
    pipeline_config.render_pass= render_pass;
    pipeline_config.pipeline_layout = pipeline_layout;

    pipeline = std::make_unique<Pipeline>(device, "assets/shaders/shader.vert.spv", "assets/shaders/shader.frag.spv", pipeline_config);
}
