#pragma once

#include <vector>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>

#include <glm/glm.hpp>

#include "engine/input/baseinput.hpp"

namespace muon {

    enum class MouseButton : uint8_t {
        Mouse1 = SDL_BUTTON_LEFT,
        Mouse2 = SDL_BUTTON_RIGHT,
        Mouse3 = SDL_BUTTON_MIDDLE,
        Mouse4 = SDL_BUTTON_X1,
        Mouse5 = SDL_BUTTON_X2,
    };

    class MouseInput : public BaseInput {
    public:
        void processEvent(SDL_Event &event) override;
        void update() override;

        bool isButtonDown(MouseButton button) const;
        glm::vec2 getCurrentPosition() const { return current_position; }
        glm::vec2 getDeltaPosition() const { return delta_position; }
    private:
        std::vector<uint8_t> buttons_down{};
        glm::vec2 previous_position{};
        glm::vec2 current_position{};
        glm::vec2 delta_position{};
    };

}
