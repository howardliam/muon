#include <cstdint>
#include <string>
#include <string_view>

#include <spdlog/spdlog.h>
#include <toml++/toml.hpp>


#include "engine/window/window.hpp"
#include "app.hpp"

void loadWindowProperties(muon::WindowProperties &window_properties) {
    auto config = toml::parse_file("config.toml");

    if (auto value = config["window"]["title"].value<std::string_view>()) {
        window_properties.title = *value;
    }

    if (auto value = config["window"]["width"].value<int32_t>()) {
        window_properties.width = *value;
    }

    if (auto value = config["window"]["height"].value<int32_t>()) {
        window_properties.height = *value;
    }
}

int main() {
    spdlog::set_level(spdlog::level::debug);


    muon::WindowProperties window_properties{};
    loadWindowProperties(window_properties);

    muon::App app{window_properties};
    app.run();
}
