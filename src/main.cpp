#include <spdlog/spdlog.h>
#include <string>

#include "engine/window/window.hpp"
#include "app.hpp"
#include "utils.hpp"

#include "cpptoml.h"

int main() {
    spdlog::set_level(spdlog::level::debug);

    auto config = cpptoml::parse_file("config.toml");
    auto window = config->get_table("window");

    WindowProperties window_properties{};
    window_properties.title = window->get_as<std::string>("title").value_or(defaults::TITLE);
    window_properties.width = window->get_as<int>("width").value_or(defaults::WIDTH);
    window_properties.height = window->get_as<int>("height").value_or(defaults::HEIGHT);

    App app{window_properties};
    app.run();
}
