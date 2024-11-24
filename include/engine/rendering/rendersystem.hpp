#pragma once

#include <vulkan/vulkan.hpp>

#include "engine/vulkan/device.hpp"
#include "engine/vulkan/pipeline.hpp"
#include "engine/vulkan/model.hpp"
#include "engine/vulkan/frameinfo.hpp"

class RenderSystem3D {
public:
    RenderSystem3D(Device &device, VkRenderPass render_pass, VkDescriptorSetLayout descriptor_set_layout);
    ~RenderSystem3D();

    RenderSystem3D(const RenderSystem3D&) = delete;
    RenderSystem3D& operator=(const RenderSystem3D&) = delete;


    // void render_game_objects(FrameInfo &frame_info, std::vector<GameObject>& game_objects);
    void render_model(FrameInfo &frame_info, Model &model);

private:
    Device &device;

    std::unique_ptr<Pipeline> pipeline;
    VkPipelineLayout pipeline_layout;

    void create_pipeline_layout(VkDescriptorSetLayout descriptor_set_layout);
    void create_pipeine(VkRenderPass render_pass);
};
