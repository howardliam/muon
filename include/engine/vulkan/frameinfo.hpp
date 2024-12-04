#pragma once

#include <vulkan/vulkan.hpp>

#include "scene/camera.hpp"

namespace muon {

    struct FrameInfo {
        int32_t frame_index;
        float frame_time;
        vk::CommandBuffer command_buffer;
        Camera &camera;
        vk::DescriptorSet descriptor_set;
    };

}
