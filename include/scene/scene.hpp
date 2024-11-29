#pragma once

#include "entt.hpp"

namespace muon {

    class Scene {
    public:
        Scene();
        ~Scene();

    private:
        entt::registry registry;
    };

}
