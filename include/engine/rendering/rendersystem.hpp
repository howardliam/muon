#pragma once

#include <vulkan/vulkan.hpp>

#include "engine/vulkan/device.hpp"
#include "engine/vulkan/pipeline.hpp"
#include "engine/vulkan/model.hpp"
#include "engine/vulkan/frameinfo.hpp"

namespace muon {
    class RenderSystem3D {
    public:
        RenderSystem3D(Device &device, vk::RenderPass render_pass, vk::DescriptorSetLayout descriptor_set_layout);
        ~RenderSystem3D();

        RenderSystem3D(const RenderSystem3D&) = delete;
        RenderSystem3D& operator=(const RenderSystem3D&) = delete;


        // void render_game_objects(FrameInfo &frame_info, std::vector<GameObject>& game_objects);
        void renderModel(FrameInfo &frame_info, Model &model);
        void renderModel(FrameInfo &frame_info, Model &model, glm::mat4 transform);

    private:
        Device &device;

        std::unique_ptr<Pipeline> pipeline;
        vk::PipelineLayout pipeline_layout;

        void createPipelineLayout(vk::DescriptorSetLayout descriptor_set_layout);
        void createPipeline(vk::RenderPass render_pass);
    };
}
