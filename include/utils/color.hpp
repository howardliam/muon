#pragma once

#include <cstdint>
#include <array>
#include <glm/glm.hpp>

namespace muon {

    namespace color {
        /**
            *  Four byte hex to RGBA
            *
            *  Expected integer structure: RR_GG_BB_AA
        */
        std::array<float, 4> hexToRgbaArray(uint32_t color);
        std::array<float, 4> hexBytesToRgbaArray(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

        /**
            *  Four byte hex to RGBA
            *
            *  Expected integer structure: RR_GG_BB_AA
        */
        glm::vec4 hexToRgbaVec(uint32_t color);
        glm::vec4 hexBytesToRgbaVec(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

        /**
            *  Three byte hex to RGB
            *
            *  Expected integer structure: XX_RR_GG_BB
        */
        std::array<float, 3> hexToRgbArray(uint32_t color);
        std::array<float, 3> hexBytesToRgbArray(uint8_t r, uint8_t g, uint8_t b);

        /**
            *  Three byte hex to RGB
            *
            *  Expected integer structure: XX_RR_GG_BB
        */
        glm::vec3 hexToRgbVec(uint32_t color);
        glm::vec3 hexBytesToRgbVec(uint8_t r, uint8_t g, uint8_t b);
    }

}
