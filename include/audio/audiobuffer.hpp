#pragma once

#include <string>
#include <AL/al.h>

namespace muon {

    class AudioBuffer {
    public:
        AudioBuffer(std::string &path);
        ~AudioBuffer();

        ALuint getBuffer() { return buffer; }

    private:
        ALuint format{};
        ALuint buffer{};
    };

}
