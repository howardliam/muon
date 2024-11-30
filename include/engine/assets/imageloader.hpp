#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace muon {

    struct PngImageProperties {
        uint32_t width{};
        uint32_t height{};
        int color_type{};
        int bit_depth{};
    };

    void readPngFile(std::string &path, std::vector<uint8_t> &image_data, PngImageProperties &properties);

}
