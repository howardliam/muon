#pragma once

#include <vulkan/vulkan.hpp>

#include "../vulkan/device.hpp"
#include "../vulkan/pipeline.hpp"
#include "../vulkan/model.hpp"

class RenderSystem {
public:
    RenderSystem(Device &device, VkRenderPass render_pass);
    ~RenderSystem();

    RenderSystem(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;


    // void render_game_objects(FrameInfo &frame_info, std::vector<GameObject>& game_objects);
    void render_model(Model &model, VkCommandBuffer command_buffer);

private:
    Device &device;

    std::unique_ptr<Pipeline> pipeline;
    VkPipelineLayout pipeline_layout;

    void create_pipeine_layout();
    void create_pipeine(VkRenderPass render_pass);
};
