#pragma once

#include <cstdint>

namespace muon {

    namespace color {
        /**
            *  Four byte hex to RGBA
            *
            *  Expected integer structure: RR_GG_BB_AA
        */
        template <typename T>
        T hexToRgba(uint32_t color);

        template <typename T>
        T hexToRgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

        /**
            *  Three byte hex to RGB
            *
            *  Expected integer structure: XX_RR_GG_BB
        */
        template <typename T>
        T hexToRgb(uint32_t color);

        template <typename T>
        T hexToRgb(uint8_t r, uint8_t g, uint8_t b);
    }

}
