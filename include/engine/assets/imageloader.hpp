#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace muon {

    struct PngProperties {
        uint32_t width{};
        uint32_t height{};
        int32_t color_type{};
        int32_t bit_depth{};
    };

    void readPngFile(const std::string &path, std::vector<uint8_t> &image_data, PngProperties &properties);

}
