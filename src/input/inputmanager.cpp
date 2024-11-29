#include "input/inputmanager.hpp"

namespace muon {

    void InputManager::processEvent(SDL_Event &event) {
        keyboard.processEvent(event);
        mouse.processEvent(event);
    }

    void InputManager::update() {
        keyboard.update();
        mouse.update();
    }

}
