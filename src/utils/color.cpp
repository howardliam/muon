#include "utils/color.hpp"

#include <climits>

namespace muon {
    namespace color {
        std::array<float, 4> hexToRgbaArray(uint32_t color) {
            return {
                static_cast<float>((color >> 0x18) & 0xFF) / UCHAR_MAX,
                static_cast<float>((color >> 0x10) & 0xFF) / UCHAR_MAX,
                static_cast<float>((color >> 0x08) & 0xFF) / UCHAR_MAX,
                static_cast<float>((color >> 0x00) & 0xFF) / UCHAR_MAX
            };
        }

        std::array<float, 4> hexBytesToRgbaArray(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
            return {
                static_cast<float>(r) / UCHAR_MAX,
                static_cast<float>(g) / UCHAR_MAX,
                static_cast<float>(b) / UCHAR_MAX,
                static_cast<float>(a) / UCHAR_MAX
            };
        }

        glm::vec4 hexToRgbaVec(uint32_t color) {
            return {
                static_cast<float>((color >> 0x18) & 0xFF) / UCHAR_MAX,
                static_cast<float>((color >> 0x10) & 0xFF) / UCHAR_MAX,
                static_cast<float>((color >> 0x08) & 0xFF) / UCHAR_MAX,
                static_cast<float>((color >> 0x00) & 0xFF) / UCHAR_MAX
            };
        }

        glm::vec4 hexBytesToRgbaVec(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
            return {
                static_cast<float>(r) / UCHAR_MAX,
                static_cast<float>(g) / UCHAR_MAX,
                static_cast<float>(b) / UCHAR_MAX,
                static_cast<float>(a) / UCHAR_MAX
            };
        }

        std::array<float, 3> hexToRgbArray(uint32_t color) {
            color &= 0xFFFFFF;
            return {
                static_cast<float>((color >> 0x10) & 0xFF) / UCHAR_MAX,
                static_cast<float>((color >> 0x08) & 0xFF) / UCHAR_MAX,
                static_cast<float>((color >> 0x00) & 0xFF) / UCHAR_MAX
            };
        }

        std::array<float, 3> hexBytesToRgbArray(uint8_t r, uint8_t g, uint8_t b) {
            return {
                static_cast<float>(r) / UCHAR_MAX,
                static_cast<float>(g) / UCHAR_MAX,
                static_cast<float>(b) / UCHAR_MAX
            };
        }

        glm::vec3 hexToRgbVec(uint32_t color) {
            color &= 0xFFFFFF;
            return {
                static_cast<float>((color >> 0x10) & 0xFF) / UCHAR_MAX,
                static_cast<float>((color >> 0x08) & 0xFF) / UCHAR_MAX,
                static_cast<float>((color >> 0x00) & 0xFF) / UCHAR_MAX
            };
        }

        glm::vec3 hexBytesToRgbVec(uint8_t r, uint8_t g, uint8_t b) {
            return {
                static_cast<float>(r) / UCHAR_MAX,
                static_cast<float>(g) / UCHAR_MAX,
                static_cast<float>(b) / UCHAR_MAX
            };
        }
    }
}
