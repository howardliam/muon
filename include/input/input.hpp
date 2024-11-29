#pragma once

#include <SDL3/SDL_events.h>

namespace muon {

    class BaseInput {
        virtual void processEvent(SDL_Event &event) = 0;
        virtual void update() = 0;
    };

}
