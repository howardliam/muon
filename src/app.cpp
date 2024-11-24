#include "app.hpp"

#include <SDL3/SDL_dialog.h>
#include <SDL3/SDL_messagebox.h>
#include <chrono>
#include <memory>

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>
#include <glm/trigonometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <SDL3/SDL_scancode.h>

#include "engine/rendering/textrenderer.hpp"
#include "engine/vulkan/buffer.hpp"
#include "engine/vulkan/descriptors.hpp"
#include "engine/vulkan/frameinfo.hpp"
#include "engine/vulkan/model.hpp"
#include "engine/vulkan/swapchain.hpp"
#include "engine/vulkan/texture.hpp"
#include "engine/rendering/rendersystem.hpp"
#include "engine/scene/camera.hpp"
#include "engine/input/inputmanager.hpp"
#include "engine/vulkan/font.hpp"

struct GlobalUbo {
    glm::mat4 projection{1.0f};
    glm::mat4 view{1.0f};
};

std::unique_ptr<Model> generate_text(Device &device, Font &font, std::string &text) {
    glm::vec3 position{0.5f, 0.5f, 0.0f};

    const auto &font_geometry = font.get_font_geometry();
    const auto &metrics = font_geometry.getMetrics();

    auto atlas = font.get_atlas();

    double x = 0.0;
    double y = 0.0;
    double fs_scale = 1.0 / (metrics.ascenderY - metrics.descenderY);

    const float space_glyph_advance = font_geometry.getGlyph(' ')->getAdvance();

    std::vector<Model::Vertex> vertices{};
    std::vector<uint32_t> indices{};
    uint32_t index_count{0};

    for (size_t i = 0; i < text.size(); i++) {
        char c = text[i];
        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            x = 0;
            y -= fs_scale * metrics.lineHeight;
            continue;
        }

        if (c == ' ') {
            float advance = space_glyph_advance;
            if (i < text.size() - 1) {
                char next_c = text[i + 1];
                double d_advance;
                font_geometry.getAdvance(d_advance, c, next_c);
                advance = (float)d_advance;
            }

            x += fs_scale * advance;
            continue;
        }

        auto glyph = font_geometry.getGlyph(c);
        if (!glyph) {
            glyph = font_geometry.getGlyph('?');
        }
        if (!glyph) {
            return nullptr;
        }

        double al, ab, ar, at;
        glyph->getQuadAtlasBounds(al, ab, ar, at);
        glm::vec2 tex_coord_min{(float)al, (float)ab};
        glm::vec2 tex_coord_max{(float)ar, (float)at};

        double pl, pb, pr, pt;
        glyph->getQuadPlaneBounds(pl, pb, pr, pt);
        glm::vec2 quad_min{(float)pl, (float)pb};
        glm::vec2 quad_max{(float)pr, (float)pt};

        quad_min *= fs_scale;
        quad_max *= fs_scale;
        quad_min += glm::vec2(x, y);
        quad_max += glm::vec2(x, y);

        /* Flip the Y for Vulkan! */
        quad_min.y = -quad_min.y;
        quad_max.y = -quad_max.y;

        float texel_width = 1.0f / atlas->get_width();
        float texel_height = 1.0f / atlas->get_height();
        tex_coord_min *= glm::vec2{texel_width, texel_height};
        tex_coord_max *= glm::vec2{texel_width, texel_height};

        vertices.push_back({ // Top left
            position * glm::vec3(quad_min.x, quad_max.y, 0.0f),
            glm::vec3{1.0f, 1.0f, 1.0f},
            glm::vec3{0.0f, 0.0f, 0.0f},
            glm::vec2{tex_coord_min.x, tex_coord_max.y},
        });

        vertices.push_back({ // Bottom left
            position * glm::vec3(quad_min, 0.0f),
            glm::vec3{1.0f, 1.0f, 1.0f},
            glm::vec3{0.0f, 0.0f, 0.0f},
            glm::vec2{tex_coord_min.x, tex_coord_min.y},
        });

        vertices.push_back({ // Bottom right
            position * glm::vec3(quad_max.x, quad_min.y, 0.0f),
            glm::vec3{1.0f, 1.0f, 1.0f},
            glm::vec3{0.0f, 0.0f, 0.0f},
            glm::vec2{tex_coord_max.x, tex_coord_min.y},
        });

        vertices.push_back({ // Top right
            position * glm::vec3(quad_max, 0.0f),
            glm::vec3{1.0f, 1.0f, 1.0f},
            glm::vec3{0.0f, 0.0f, 0.0f},
            glm::vec2{tex_coord_max.x, tex_coord_max.y},
        });

        indices.push_back(index_count + 0);
        indices.push_back(index_count + 1);
        indices.push_back(index_count + 2);
        indices.push_back(index_count + 2);
        indices.push_back(index_count + 3);
        indices.push_back(index_count + 0);

        index_count+= 4;

        if (i < text.size() - 1) {
            double advance = glyph->getAdvance();
            char next_c = text[i + 1];
            font_geometry.getAdvance(advance, c, next_c);
            x += fs_scale * advance;
        }
    }

    Model::Builder builder{vertices, indices};
    return std::make_unique<Model>(device, builder);
}

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
    std::string font_path = "assets/fonts/OpenSans-Regular.ttf";
    Font font{font_path, device};
    auto atlas = font.get_atlas();

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
        // auto image_info = texture.descriptor_info();
        auto image_info = atlas->descriptor_info();

        DescriptorWriter(*global_set_layout, *global_pool)
            .write_to_buffer(0, &buffer_info)
            .write_image(1, &image_info)
            .build(global_descriptor_sets[i]);
    }

    RenderSystem3D render_system{device, renderer.get_swapchain_render_pass(), global_set_layout->get_descriptor_set_layout()};
    TextRenderer text_renderer{device, renderer.get_swapchain_render_pass(), global_set_layout->get_descriptor_set_layout()};

    glm::vec3 camera_pos = {0.0f, 0.0f, 0.0f};
    Camera camera{};
    camera.look_at(camera_pos, {0.0f, 0.0f, -1.0f});

    std::unique_ptr model = create_model_from_file(device, "assets/models/quad.obj");

    std::string text = "Hello, World";
    std::unique_ptr text_model = generate_text(device, font, text);

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

        camera.set_perspective_projection(glm::radians(90.0f), renderer.get_aspect_ratio(), 0.01f, 1000.0f);

        renderer.set_clear_colour({0.05f, 0.05f, 0.05f, 1.0f});
        if (const auto command_buffer = renderer.begin_frame()) {
            const int frame_index = renderer.get_frame_index();

            GlobalUbo global_ubo{};
            global_ubo.projection = camera.get_projection();
            global_ubo.view = camera.get_view();
            ubo_buffers[frame_index]->write_to_buffer(&global_ubo);
            ubo_buffers[frame_index]->flush();

            renderer.begin_swapchain_render_pass(command_buffer);

            std::string fps_text = std::to_string(static_cast<int>(1.0f / frame_time)) + " FPS";
            text_model = generate_text(device, font, fps_text);

            FrameInfo frame_info{
                frame_index,
                frame_time,
                command_buffer,
                camera,
                global_descriptor_sets[frame_index]
            };
            render_system.render_model(frame_info, *model);
            render_system.render_model(frame_info, *text_model);

            renderer.end_swapchain_render_pass(command_buffer);
            renderer.end_frame();
        }

        input_manager.update();
    }

    vkDeviceWaitIdle(device.get_device());
}
