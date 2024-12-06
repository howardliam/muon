#pragma once

#include <string>
#include <cstdint>
#include <AL/al.h>

namespace muon {

    class AudioBuffer {
    public:
        AudioBuffer(std::string &path);
        ~AudioBuffer();

        uint32_t getBuffer() { return buffer; }

    private:
        uint32_t format{};
        uint32_t buffer{};
    };

}
