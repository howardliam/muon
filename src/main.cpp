#include <spdlog/spdlog.h>


#include "engine/window/window.hpp"
#include "app.hpp"

int main() {
    spdlog::set_level(spdlog::level::debug);

    WindowProperties window_properties{};
    window_properties.title = "Hello, Vulkan!";

    App app{window_properties};
    app.run();
}
