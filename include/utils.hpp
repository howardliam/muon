#pragma once

namespace muon {

    namespace exitcode {
        constexpr int SUCCESS = 0;
        constexpr int FAILURE = 1;
    }

    namespace defaults {
        constexpr int WIDTH = 1280;
        constexpr int HEIGHT = 720;
        constexpr char TITLE[] = "Vulkan SDL3 Window";
        constexpr char ICON_PATH[] = "assets/textures/icon.png";
    }

}
