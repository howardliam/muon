#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <AL/alc.h>

#include "resource.hpp"
#include "source.hpp"

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    AudioSource get_audio_source(std::string &filename);

private:
    ALCdevice *device;
    ALCcontext *context;

    std::unordered_map<std::string, std::shared_ptr<AudioResource>> audio_resources{};

    std::shared_ptr<AudioResource> get_audio_resource(std::string &filename);
};
