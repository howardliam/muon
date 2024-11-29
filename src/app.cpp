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

namespace muon {

    struct GlobalUbo {
        glm::mat4 projection{1.0f};
        glm::mat4 view{1.0f};
    };

    std::unique_ptr<Model> generateText(Device &device, Font &font, std::string &text) {
        const auto &font_geometry = font.getFontGeometry();
        const auto &metrics = font_geometry.getMetrics();

        auto atlas = font.getAtlas();

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

            float texel_width = 1.0f / atlas->getWidth();
            float texel_height = 1.0f / atlas->getHeight();
            tex_coord_min *= glm::vec2{texel_width, texel_height};
            tex_coord_max *= glm::vec2{texel_width, texel_height};

            vertices.push_back({ // Top left
                glm::vec3{quad_min.x, quad_max.y, 0.0f},
                glm::vec3{1.0f, 1.0f, 1.0f},
                glm::vec3{0.0f, 0.0f, 0.0f},
                glm::vec2{tex_coord_min.x, tex_coord_max.y},
            });

            vertices.push_back({ // Bottom left
                glm::vec3{quad_min, 0.0f},
                glm::vec3{1.0f, 1.0f, 1.0f},
                glm::vec3{0.0f, 0.0f, 0.0f},
                glm::vec2{tex_coord_min.x, tex_coord_min.y},
            });

            vertices.push_back({ // Bottom right
                glm::vec3{quad_max.x, quad_min.y, 0.0f},
                glm::vec3{1.0f, 1.0f, 1.0f},
                glm::vec3{0.0f, 0.0f, 0.0f},
                glm::vec2{tex_coord_max.x, tex_coord_min.y},
            });

            vertices.push_back({ // Top right
                glm::vec3{quad_max, 0.0f},
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
            .setMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .build();
    }

    App::~App() {
        spdlog::info("Shutting down");
    }

    void App::run() {
        std::string font_path = "assets/fonts/OpenSans-Regular.ttf";
        Font font{font_path, device};
        auto atlas = font.getAtlas();

        InputManager input_manager;
        window.bindInputManager(&input_manager);

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
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

        Texture texture{device, "assets/textures/icon.png"};

        std::vector<VkDescriptorSet> global_descriptor_sets(Swapchain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < global_descriptor_sets.size(); i++) {
            auto buffer_info = ubo_buffers[i]->descriptorInfo();
            // auto image_info = texture.descriptorInfo();
            auto image_info = atlas->descriptorInfo();

            DescriptorWriter(*global_set_layout, *global_pool)
                .writeToBuffer(0, &buffer_info)
                .writeImage(1, &image_info)
                .build(global_descriptor_sets[i]);
        }

        RenderSystem3D render_system{device, renderer.getSwapchainRenderPass(), global_set_layout->getDescriptorSetLayout()};

        glm::vec3 camera_pos = {0.0f, 0.0f, 0.0f};
        Camera camera{};
        camera.lookAt(camera_pos, {0.0f, 0.0f, -1.0f});

        std::unique_ptr model = Model::fromFile(device, "assets/models/quad.obj");

        std::unique_ptr<Model> text_model = nullptr;

        auto current_time = std::chrono::high_resolution_clock::now();
        float frame_time;

        while (window.isOpen()) {
            window.pollEvents();

            if (input_manager.getKeyboard().isKeyDown(SDL_SCANCODE_ESCAPE)) {
                window.setToClose();
            }

            if (input_manager.getMouse().isButtonDown(MouseButton::Mouse1)) {
                window.setTitle("Hello");
            } else if (input_manager.getMouse().isButtonDown(MouseButton::Mouse2)) {
                window.setTitle("World");
            }

            auto new_time = std::chrono::high_resolution_clock::now();
            frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
            current_time = new_time;

            // window.setTitle(std::to_string(static_cast<int>(1 / frame_time)) + " FPS");

            camera.setPerspectiveProjection(glm::radians(90.0f), renderer.getAspectRatio(), 0.01f, 1000.0f);
            // camera.setOrthographicProjection(-renderer.getAspectRatio(), renderer.getAspectRatio(), -1, 1);

            renderer.setClearColor({0.05f, 0.05f, 0.05f, 1.0f});
            if (const auto command_buffer = renderer.beginFrame()) {
                const int frame_index = renderer.getFrameIndex();

                GlobalUbo global_ubo{};
                global_ubo.projection = camera.getProjection();
                global_ubo.view = camera.getView();
                ubo_buffers[frame_index]->writeToBuffer(&global_ubo);
                ubo_buffers[frame_index]->flush();

                renderer.beginSwapchainRenderPass(command_buffer);

                // std::string fps_text = std::to_string(static_cast<int>(1.0f / frame_time)) + " FPS";
                // text_model = generateText(device, font, fps_text);

                auto mouse_pos = input_manager.getMouse().getCurrentPosition();
                std::string pos_text = std::to_string(mouse_pos.x) + "\n" + std::to_string(mouse_pos.y);
                text_model = generateText(device, font, pos_text);


                FrameInfo frame_info{
                    frame_index,
                    frame_time,
                    command_buffer,
                    camera,
                    global_descriptor_sets[frame_index]
                };
                // render_system.renderModel(frame_info, *model);
                render_system.renderModel(frame_info, *text_model);

                renderer.endSwapchainRenderPass(command_buffer);
                renderer.endFrame();
            }

            input_manager.update();
        }

        vkDeviceWaitIdle(device.getDevice());
    }

}
