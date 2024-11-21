#pragma once

#include <vector>
#include <string>

#include <AL/al.h>

class AudioResource {
public:
    AudioResource(std::string &filename);
    ~AudioResource();

    bool loaded_successfully() { return loaded; }
    void play();

private:
    bool loaded = false;
    std::vector<short> data{};
    int sample_rate{};
    int channels{};

    ALenum format{};
    ALuint buffer{};
    ALuint source{};

    void load_ogg(std::string &filename);
};
