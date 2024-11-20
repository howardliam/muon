#include "app.hpp"

#include <chrono>
#include <memory>

#include <vulkan/vulkan.hpp>
#include <glm/trigonometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <SDL3/SDL_scancode.h>

#include "engine/input/inputmanager.hpp"
#include "engine/vulkan/buffer.hpp"
#include "engine/vulkan/descriptors.hpp"
#include "engine/vulkan/frameinfo.hpp"
#include "engine/vulkan/model.hpp"
#include "engine/vulkan/swapchain.hpp"
#include "engine/vulkan/texture.hpp"

#include "engine/rendering/rendersystem.hpp"

#include "engine/scene/camera.hpp"

struct GlobalUbo {
    glm::mat4 projection{1.0f};
    glm::mat4 view{1.0f};
};

App::App(WindowProperties &properties) : properties{properties} {
    spdlog::info("Starting up");

    global_pool = DescriptorPool::Builder(device)
        .set_max_sets(Swapchain::MAX_FRAMES_IN_FLIGHT)
        .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT)
        .add_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Swapchain::MAX_FRAMES_IN_FLIGHT)
        .build();
}

App::~App() {
    spdlog::info("Shutting down");
}

void App::run() {
    InputManager input_manager;
    window.bind_input_manager(&input_manager);

    std::vector<std::unique_ptr<Buffer>> ubo_buffers(Swapchain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < ubo_buffers.size(); i++) {
        ubo_buffers[i] = std::make_unique<Buffer>(
            device,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        ubo_buffers[i]->map();
    }

    auto global_set_layout = DescriptorSetLayout::Builder(device)
        .add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();

    Texture texture{device, "assets/textures/icon.png"};

    std::vector<VkDescriptorSet> global_descriptor_sets(Swapchain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < global_descriptor_sets.size(); i++) {
        auto buffer_info = ubo_buffers[i]->descriptor_info();
        auto image_info = texture.descriptor_info();

        DescriptorWriter(*global_set_layout, *global_pool)
            .write_to_buffer(0, &buffer_info)
            .write_image(1, &image_info)
            .build(global_descriptor_sets[i]);
    }

    RenderSystem render_system{device, renderer.get_swapchain_render_pass(), global_set_layout->get_descriptor_set_layout()};

    glm::vec3 camera_pos = {0.0f, 0.0f, 0.0f};
    Camera camera{};
    camera.look_at(camera_pos, {0.0f, 0.0f, -1.0f});

    std::unique_ptr model = create_model_from_file(device, "assets/models/cube.obj");

    auto current_time = std::chrono::high_resolution_clock::now();
    float frame_time;

    while (window.is_open()) {
        window.poll_events();

        if (input_manager.is_key_pressed(SDL_SCANCODE_ESCAPE)) {
            window.set_to_close();
        }

        auto new_time = std::chrono::high_resolution_clock::now();
        frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
        current_time = new_time;

        // window.set_title(std::to_string(static_cast<int>(1 / frame_time)) + " FPS");

        camera.set_perspective_projection(glm::radians(45.0f), renderer.get_aspect_ratio(), 0.01f, 1000.0f);

        if (const auto command_buffer = renderer.begin_frame()) {
            const int frame_index = renderer.get_frame_index();

            GlobalUbo global_ubo{};
            global_ubo.projection = camera.get_projection();
            global_ubo.view = camera.get_view();
            ubo_buffers[frame_index]->write_to_buffer(&global_ubo);
            ubo_buffers[frame_index]->flush();

            renderer.begin_swapchain_render_pass(command_buffer);

            FrameInfo frame_info{
                frame_index,
                frame_time,
                command_buffer,
                camera,
                global_descriptor_sets[frame_index]
            };
            render_system.render_model(frame_info, *model);

            renderer.end_swapchain_render_pass(command_buffer);
            renderer.end_frame();
        }

        input_manager.update();
    }

    vkDeviceWaitIdle(device.get_device());
}
