#pragma once

#include <vector>

#include <SDL3/SDL_scancode.h>

#include "engine/input/baseinput.hpp"

namespace muon {

    class KeyboardInput : public BaseInput {
    public:
        void processEvent(SDL_Event &event) override;
        void update() override;

        bool isKeyDown(SDL_Scancode key) const;
        bool isKeyUp(SDL_Scancode key) const;

    private:
        std::vector<SDL_Scancode> keys_down{};
        std::vector<SDL_Scancode> keys_up{};
    };

}
