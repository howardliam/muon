#include <string>
#include <string_view>

#include <spdlog/spdlog.h>

#include <utils.hpp>
#include <engine/window/window.hpp>
#include <engine/assets/audioloader.hpp>
#include <audio/soundsystem.hpp>
#include <audio/audiobuffer.hpp>
#include <toml++/toml.hpp>

#include "app.hpp"
#include "audio/audiosource.hpp"

int main() {
    spdlog::set_level(spdlog::level::debug);

    // muon::SoundSystem sound_system;
    // std::string path = "assets/audio/break-window.ogg";
    // muon::AudioBuffer buffer{path};
    // muon::AudioSource source{buffer};
    // source.play();

    auto config = toml::parse_file("config.toml");
    std::string_view title = config["window"]["title"].value_or(muon::defaults::TITLE);
    int width = config["window"]["width"].value_or(muon::defaults::WIDTH);
    int height = config["window"]["height"].value_or(muon::defaults::HEIGHT);

    muon::WindowProperties window_properties{};
    window_properties.title = title;
    window_properties.width = width;
    window_properties.height = height;

    muon::App app{window_properties};
    app.run();
}
