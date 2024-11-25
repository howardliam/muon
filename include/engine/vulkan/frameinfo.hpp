#pragma once

#include <vulkan/vulkan.hpp>

#include "engine/scene/camera.hpp"

namespace muon {

    struct FrameInfo {
        int frame_index;
        float frame_time;
        VkCommandBuffer command_buffer;
        Camera &camera;
        VkDescriptorSet descriptor_set;
    };

}
