#include <audio/soundsystem.hpp>

#include <spdlog/spdlog.h>
#include <utils.hpp>

namespace muon {

    SoundSystem::SoundSystem() {
        device = alcOpenDevice(nullptr);
        if (device == nullptr) {
            spdlog::error("Unable to open audio device, exiting");
            exit(exitcode::FAILURE);
        }

        context = alcCreateContext(device, nullptr);
        if (context == nullptr) {
            spdlog::error("Unable to create audio context, exiting");
            exit(exitcode::FAILURE);
        }
        alcMakeContextCurrent(context);
    }

    SoundSystem::~SoundSystem() {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context);
        alcCloseDevice(device);
    }

}
