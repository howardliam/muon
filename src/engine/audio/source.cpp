#include "source.hpp"
#include <AL/al.h>

AudioSource::AudioSource(AudioResource &resource) {
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, resource.get_buffer());
}

AudioSource::~AudioSource() {
    alDeleteSources(1, &source);
}

void AudioSource::set_volume(float volume) {
    alSourcef(source, AL_GAIN, volume);
}

void AudioSource::set_position(glm::vec3 position) {
    alSource3f(source, AL_POSITION, position.x, position.y, position.z);
}

void AudioSource::set_velocity(glm::vec3 velocity) {
    alSource3f(source, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}
