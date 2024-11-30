#pragma once

#include <AL/alc.h>

namespace muon {

    class SoundSystem {
    public:
        SoundSystem();
        ~SoundSystem();

    private:
        ALCdevice *device;
        ALCcontext *context;
    };

}
