#pragma once

#include <audio/audiobuffer.hpp>
#include <cstdint>
#include <AL/al.h>

namespace muon {

    class AudioSource {
    public:
        AudioSource(AudioBuffer &buffer);
        ~AudioSource();

        void play() { alSourcePlay(source); }
        void pause() { alSourcePause(source); }
        void resume() { alSourcePlay(source); }
        void stop() { alSourceStop(source); }

    private:
        uint32_t source{};
    };

}
