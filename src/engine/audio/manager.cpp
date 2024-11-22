#include "manager.hpp"

#include <fstream>
#include <spdlog/spdlog.h>

#include "../../utils.hpp"

bool file_exists(std::string &path) {
    std::ifstream file{path};
    return file.good();
}

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

AudioSource AudioManager::get_audio_source(std::string &filename) {
    auto resource = get_audio_resource(filename);

    return AudioSource{*resource};
}

std::shared_ptr<AudioResource> AudioManager::get_audio_resource(std::string &filename) {
    if (!file_exists(filename)) {
        spdlog::warn("{} not found", filename);
        return nullptr;
    }

    auto resource = audio_resources[filename];
    if (resource != nullptr) {
        return resource;
    }

    resource = std::make_shared<AudioResource>(filename);
    audio_resources[filename] = resource;

    return resource;
}
