#include "engine/input/keyboard.hpp"

#include <algorithm>

namespace muon {

    void KeyboardInput::processEvent(SDL_Event &event) {
        if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
            keys_down.push_back(event.key.scancode);
        } else if (event.type == SDL_EVENT_KEY_UP) {
            keys_up.push_back(event.key.scancode);
        }
    }

    void KeyboardInput::update() {
        keys_down.clear();
        keys_up.clear();
    }

    bool KeyboardInput::isKeyDown(SDL_Scancode key) const {
        return std::find(keys_down.begin(), keys_down.end(), key) != keys_down.end();
    }

    bool KeyboardInput::isKeyUp(SDL_Scancode key) const {
        return std::find(keys_up.begin(), keys_up.end(), key) != keys_up.end();
    }

}
