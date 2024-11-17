#include "window.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>

#include "../../stb/stb_image.h"

Window::Window(WindowProperties &properties) : properties{properties} {
    initialise_sdl();
    initialise_window();
}

Window::~Window() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Window::poll_events() {
    SDL_Event event;
        while (SDL_PollEvent(&event)) {
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

void Window::set_icon(const char *icon_path) {
    int width, height, channels;
    unsigned char *image_data = stbi_load(icon_path, &width, &height, &channels, 4);

    SDL_Surface *surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA32, image_data, width * 4);
    SDL_SetWindowIcon(window, surface);

    stbi_image_free(image_data);
}

void Window::create_surface(VkInstance instance, VkSurfaceKHR *surface) {
    if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, surface)) {
        spdlog::error("Failed to create window surface");
        exit(exitcode::FAILURE);
    }
}

void Window::initialise_sdl() {
    spdlog::debug("Begin initialising SDL");
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        auto err = SDL_GetError();
        spdlog::error("Failed to initialise SDL: {}", err);
        exit(exitcode::FAILURE);
    }
    spdlog::debug("Finished initialising SDL");
}

void Window::initialise_window() {
    spdlog::debug("Begin initialising window");
    auto flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow(properties.title.c_str(), properties.width, properties.height, flags);

    if (window == nullptr) {
        auto err = SDL_GetError();
        spdlog::error("Failed to create window: {}", err);
        exit(exitcode::FAILURE);
    }

    auto pos = SDL_WINDOWPOS_CENTERED;
    SDL_SetWindowPosition(window, pos, pos);

    set_icon(defaults::ICON_PATH);

    spdlog::debug("Finished initialising window");
}
