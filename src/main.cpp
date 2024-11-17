#include <spdlog/spdlog.h>

#include "engine/rendering/rendersystem.hpp"
#include "engine/window/window.hpp"
#include "engine/vulkan/device.hpp"
#include "engine/vulkan/renderer.hpp"
#include "engine/vulkan/model.hpp"

int main() {
    spdlog::info("Starting up");
    spdlog::set_level(spdlog::level::debug);

    WindowProperties window_properties{};
    Window window{window_properties};

    Device device{window};
    Renderer renderer{window, device};
    RenderSystem render_system{device, renderer.get_swapchain_render_pass()};

    std::unique_ptr model = create_model_from_file(device, "assets/models/cube.obj");

    while (window.is_open()) {
        window.poll_events();

        const auto command_buffer = renderer.begin_frame();
        renderer.begin_swapchain_render_pass(command_buffer);

        render_system.render_model(*model, command_buffer);

        renderer.end_swapchain_render_pass(command_buffer);
        renderer.end_frame();
    }

    vkDeviceWaitIdle(device.get_device());
}
