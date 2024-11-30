#include <audio/audiosource.hpp>

namespace muon {

    AudioSource::AudioSource(AudioBuffer &buffer) {
        alGenSources(1, &source);
        alSourcei(source, AL_BUFFER, buffer.getBuffer());
    }

    AudioSource::~AudioSource() {
        alDeleteSources(1, &source);
    }

}
