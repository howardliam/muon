#include "app.hpp"

#include <chrono>

#include "engine/rendering/rendersystem.hpp"
#include "engine/vulkan/model.hpp"
#include "engine/vulkan/texture.hpp"

App::App(WindowProperties &properties) : properties{properties} {
    spdlog::info("Starting up");
}

App::~App() = default;

void App::run() {
    RenderSystem render_system{device, renderer.get_swapchain_render_pass()};

    std::unique_ptr model = create_model_from_file(device, "assets/models/cube.obj");

    Texture texture{device, "assets/textures/icon.png"};

    auto current_time = std::chrono::high_resolution_clock::now();
    float frame_time;

    while (window.is_open()) {
        window.poll_events();

        auto new_time = std::chrono::high_resolution_clock::now();
        frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
        current_time = new_time;

        window.set_title(std::to_string(static_cast<int>(1 / frame_time)) + " FPS");

        const auto command_buffer = renderer.begin_frame();
        renderer.begin_swapchain_render_pass(command_buffer);

        render_system.render_model(*model, command_buffer);

        renderer.end_swapchain_render_pass(command_buffer);
        renderer.end_frame();
    }

    vkDeviceWaitIdle(device.get_device());
}
