#include "audio/audiobuffer.hpp"

#include <spdlog/spdlog.h>
#include <vector>
#include "engine/assets/audioloader.hpp"

namespace muon {

    AudioBuffer::AudioBuffer(std::string &path) {
        OggProperties properties;
        std::vector<short> audio_data;
        loadOggFile(path, audio_data, properties);

        if (properties.channels == 1) {
            format = AL_FORMAT_MONO16;
        } else if (properties.channels == 2) {
            format = AL_FORMAT_STEREO16;
        }

        alGenBuffers(1, &buffer);
        alBufferData(buffer, format, audio_data.data(), audio_data.size() * sizeof(short), properties.sample_rate);
    }

    AudioBuffer::~AudioBuffer() {
        alDeleteBuffers(1, &buffer);
    }

}
