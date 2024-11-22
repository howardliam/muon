#pragma once

#include <AL/al.h>
#include <glm/glm.hpp>

#include "resource.hpp"

class AudioSource {
public:
    AudioSource(AudioResource &resource);
    ~AudioSource();

    void play() { alSourcePlay(source); }
    void pause() { alSourcePause(source); }
    void resume() { alSourcePlay(source); }
    void stop() { alSourceStop(source); }

    void set_volume(float volume);
    void set_position(glm::vec3 position);
    void set_velocity(glm::vec3 velocity);

private:
    ALuint source{};
};
