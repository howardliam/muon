#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace muon {

    struct PngProperties {
        uint32_t width{};
        uint32_t height{};
        int color_type{};
        int bit_depth{};
    };

    void readPngFile(const std::string &path, std::vector<uint8_t> &image_data, PngProperties &properties);

}
