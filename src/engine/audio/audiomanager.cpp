#include "audiomanager.hpp"

#include <spdlog/spdlog.h>

#include "../../utils.hpp"

AudioManager::AudioManager() {
    device = alcOpenDevice(nullptr);
    if (device == nullptr) {
        spdlog::error("Unable to open audio device");
        exit(exitcode::FAILURE);
    }

    context = alcCreateContext(device, nullptr);
    if (context == nullptr) {
        spdlog::error("Unable to create audio context");
        exit(exitcode::FAILURE);
    }
    alcMakeContextCurrent(context);
}

AudioManager::~AudioManager() {
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

std::shared_ptr<AudioResource> AudioManager::get_audio_resource(std::string &filename) {
    auto resource = audio_resources[filename];
    if (resource != nullptr) {
        return resource;
    }

    resource = std::make_shared<AudioResource>(filename);
    audio_resources[filename] = resource;

    return resource;
}
