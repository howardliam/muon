#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <AL/alc.h>

#include "audioresource.hpp"

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    std::shared_ptr<AudioResource> get_audio_resource(std::string &filename);

private:
    ALCdevice *device;
    ALCcontext *context;

    std::unordered_map<std::string, std::shared_ptr<AudioResource>> audio_resources{};
};
