#pragma once

#include <vulkan/vulkan.hpp>

struct FrameInfo {
    int frame_index;
    float frame_time;
    VkCommandBuffer command_buffer;
    /* Camera class */
};
