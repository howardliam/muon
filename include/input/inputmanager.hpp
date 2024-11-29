#pragma once

#include <SDL3/SDL_events.h>

#include "input/keyboard.hpp"
#include "input/mouse.hpp"

namespace muon {

    class InputManager {
    public:
        void processEvent(SDL_Event &event);
        void update();

        const KeyboardInput &getKeyboard() const { return keyboard; }
        const MouseInput &getMouse() const { return mouse; }
    private:
        KeyboardInput keyboard;
        MouseInput mouse;
    };

}
