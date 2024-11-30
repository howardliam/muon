#include <engine/window/window.hpp>
#include <utils.hpp>
#include <engine/assets/imageloader.hpp>

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <spdlog/spdlog.h>

namespace muon {

    Window::Window(WindowProperties &properties) : properties{properties} {
        initializeSdl();
        initializeWindow();
    }

    Window::~Window() {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void Window::pollEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (input_manager != nullptr) {
                input_manager->processEvent(event);
            }

            if (event.type == SDL_EVENT_QUIT) {
                properties.open = false;
                break;
            } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                properties.width = event.window.data1;
                properties.height = event.window.data2;
                resized = true;
            }
        }
    }

    void Window::setIcon(std::string &icon_path) {
        PngProperties properties{};
        std::vector<uint8_t> image_data;
        readPngFile(icon_path, image_data, properties);

        SDL_Surface *surface = SDL_CreateSurfaceFrom(
            properties.width, properties.height, SDL_PIXELFORMAT_RGBA32, image_data.data(), properties.width * 4);
        SDL_SetWindowIcon(window, surface);
    }

    void Window::createSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, surface)) {
            spdlog::error("Failed to create window surface");
            exit(exitcode::FAILURE);
        }
    }

    void Window::bindInputManager(InputManager *input_manager) {
        this->input_manager = input_manager;
    }

    void Window::initializeSdl() {
        spdlog::debug("Begin initialising SDL");
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            auto err = SDL_GetError();
            spdlog::error("Failed to initialise SDL: {}", err);
            exit(exitcode::FAILURE);
        }
        spdlog::debug("Finished initialising SDL");
    }

    void Window::initializeWindow() {
        spdlog::debug("Begin initialising window");
        auto flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
        window = SDL_CreateWindow(properties.title.c_str(), properties.width, properties.height, flags);

        if (window == nullptr) {
            auto err = SDL_GetError();
            spdlog::error("Failed to initialise window: {}", err);
            exit(exitcode::FAILURE);
        }

        auto pos = SDL_WINDOWPOS_CENTERED;
        SDL_SetWindowPosition(window, pos, pos);

        std::string default_icon = defaults::ICON_PATH;
        setIcon(default_icon);

        spdlog::debug("Finished initialising window");
    }

}
