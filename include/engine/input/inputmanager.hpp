#pragma once

#include "engine/input/keyboard.hpp"
#include "engine/input/mouse.hpp"

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
