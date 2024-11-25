#pragma once

#include <string>

#include <vulkan/vulkan.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

#include "../../utils.hpp"
#include "../input/inputmanager.hpp"

namespace muon {

    struct WindowProperties {
        int width{defaults::WIDTH};
        int height{defaults::HEIGHT};
        std::string title{defaults::TITLE};
        bool open{true};
    };

    class Window {
    public:
        Window(WindowProperties &properties);
        ~Window();

        Window(const Window &) = delete;
        Window& operator=(const Window &) = delete;

        void poll_events();

        VkExtent2D get_extent() const { return { static_cast<uint32_t>(properties.width), static_cast<uint32_t>(properties.height) }; }
        bool is_open() const { return properties.open; }
        void set_to_close() { properties.open = false; }
        void set_icon(const char *icon_path);

        SDL_Window *get_window() const { return window; }
        void set_title(std::string title) { SDL_SetWindowTitle(window, title.c_str()); properties.title = title; }

        bool was_resized() const { return resized; }
        void reset_resized() { resized = false; }

        void create_surface(VkInstance instance, VkSurfaceKHR *surface);

        void bind_input_manager(InputManager *input_manager);

    private:
        WindowProperties properties;

        SDL_Window *window;

        bool resized{false};

        InputManager *input_manager{nullptr};

        void initialise_sdl();
        void initialise_window();
    };

}
