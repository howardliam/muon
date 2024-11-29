#include "engine/input/mouse.hpp"

#include <algorithm>

namespace muon {

    void MouseInput::processEvent(SDL_Event &event) {
        SDL_GetMouseState(&current_position.x, &current_position.y);

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            buttons_down.push_back(event.button.button);
        }
    }

    void MouseInput::update() {
        buttons_down.clear();

        previous_position = current_position;
        delta_position = current_position - previous_position;
    }

    bool MouseInput::isButtonDown(MouseButton button) const {
        uint8_t button_byte = static_cast<uint8_t>(button);
        return std::find(buttons_down.begin(), buttons_down.end(), button_byte) != buttons_down.end();
    }

}
