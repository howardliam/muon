#include "utils/color.hpp"

#include <array>
#include <glm/glm.hpp>

namespace muon {
    namespace color {
        template <>
        std::array<float, 4> hexToRgba<std::array<float, 4>>(uint32_t color) {
            return {
                static_cast<float>((color >> 0x18) & 0xFF) / 0xFF,
                static_cast<float>((color >> 0x10) & 0xFF) / 0xFF,
                static_cast<float>((color >> 0x08) & 0xFF) / 0xFF,
                static_cast<float>((color >> 0x00) & 0xFF) / 0xFF
            };
        }

        template <>
        glm::vec4 hexToRgba<glm::vec4>(uint32_t color) {
            return {
                static_cast<float>((color >> 0x18) & 0xFF) / 0xFF,
                static_cast<float>((color >> 0x10) & 0xFF) / 0xFF,
                static_cast<float>((color >> 0x08) & 0xFF) / 0xFF,
                static_cast<float>((color >> 0x00) & 0xFF) / 0xFF
            };
        }

        template <>
        std::array<float, 4> hexToRgba<std::array<float, 4>>(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
            return {
                static_cast<float>(r) / 0xFF,
                static_cast<float>(g) / 0xFF,
                static_cast<float>(b) / 0xFF,
                static_cast<float>(a) / 0xFF
            };
        }

        template <>
        glm::vec4 hexToRgba<glm::vec4>(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
            return {
                static_cast<float>(r) / 0xFF,
                static_cast<float>(g) / 0xFF,
                static_cast<float>(b) / 0xFF,
                static_cast<float>(a) / 0xFF
            };
        }

        template <>
        std::array<float, 3> hexToRgb<std::array<float, 3>>(uint32_t color) {
            return {
                static_cast<float>((color >> 0x10) & 0xFF) / 0xFF,
                static_cast<float>((color >> 0x08) & 0xFF) / 0xFF,
                static_cast<float>((color >> 0x00) & 0xFF) / 0xFF
            };
        }

        template <>
        glm::vec3 hexToRgb<glm::vec3>(uint32_t color) {
            return {
                static_cast<float>((color >> 0x10) & 0xFF) / 0xFF,
                static_cast<float>((color >> 0x08) & 0xFF) / 0xFF,
                static_cast<float>((color >> 0x00) & 0xFF) / 0xFF
            };
        }

        template <>
        std::array<float, 3> hexToRgba<std::array<float, 3>>(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
            return {
                static_cast<float>(r) / 0xFF,
                static_cast<float>(g) / 0xFF,
                static_cast<float>(b) / 0xFF
            };
        }

        template <>
        glm::vec3 hexToRgb<glm::vec3>(uint8_t r, uint8_t g, uint8_t b) {
            return {
                static_cast<float>(r) / 0xFF,
                static_cast<float>(g) / 0xFF,
                static_cast<float>(b) / 0xFF
            };
        }
    }
}
