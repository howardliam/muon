#pragma once

#include <string>

#include <vulkan/vulkan.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

#include "utils.hpp"
#include "input/inputmanager.hpp"

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

        void pollEvents();

        VkExtent2D getExtent() const { return { static_cast<uint32_t>(properties.width), static_cast<uint32_t>(properties.height) }; }
        bool isOpen() const { return properties.open; }
        void setToClose() { properties.open = false; }
        void setIcon(const char *icon_path);

        SDL_Window *getWindow() const { return window; }
        void setTitle(std::string title) { SDL_SetWindowTitle(window, title.c_str()); properties.title = title; }

        bool wasResized() const { return resized; }
        void resetResized() { resized = false; }

        void createSurface(VkInstance instance, VkSurfaceKHR *surface);

        void bindInputManager(InputManager *input_manager);

    private:
        WindowProperties properties;

        SDL_Window *window;

        bool resized{false};

        InputManager *input_manager{nullptr};

        void initializeSdl();
        void initializeWindow();
    };

}
