cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)

set(PROJ_NAME muon)

project(${PROJ_NAME} VERSION 0.1.0 LANGUAGES C CXX)

find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)
find_package(glm REQUIRED)
find_package(spdlog REQUIRED)

find_package(OpenAL REQUIRED)

include_directories(
    libs/stb
    libs/tinyobjloader
    libs/entt
)

set(ENGINE_SRC
    # Audio
    src/engine/audio/audiolistener.cpp
    src/engine/audio/audiolistener.hpp
    src/engine/audio/audiomanager.cpp
    src/engine/audio/audiomanager.hpp
    src/engine/audio/audioresource.cpp
    src/engine/audio/audioresource.hpp
    src/engine/audio/stb_vorbis.c

    # ECS

    # Input
    src/engine/input/inputmanager.cpp
    src/engine/input/inputmanager.hpp

    # Rendering systems
    src/engine/rendering/rendersystem.cpp
    src/engine/rendering/rendersystem.hpp

    # Scene
    src/engine/scene/camera.cpp
    src/engine/scene/camera.hpp
    src/engine/scene/gameobject.cpp
    src/engine/scene/gameobject.hpp

    # Vulkan
    src/engine/vulkan/buffer.cpp
    src/engine/vulkan/buffer.hpp
    src/engine/vulkan/descriptors.cpp
    src/engine/vulkan/descriptors.hpp
    src/engine/vulkan/device.cpp
    src/engine/vulkan/device.hpp
    src/engine/vulkan/frameinfo.hpp
    src/engine/vulkan/model.cpp
    src/engine/vulkan/model.hpp
    src/engine/vulkan/pipeline.cpp
    src/engine/vulkan/pipeline.hpp
    src/engine/vulkan/renderer.cpp
    src/engine/vulkan/renderer.hpp
    src/engine/vulkan/swapchain.cpp
    src/engine/vulkan/swapchain.hpp
    src/engine/vulkan/texture.cpp
    src/engine/vulkan/texture.hpp

    # Window
    src/engine/window/window.cpp
    src/engine/window/window.hpp
)

set(PROJ_SRC
    src/main.cpp
    src/utils.hpp

    src/app.cpp
    src/app.hpp
)

add_executable(${PROJ_NAME}
    ${ENGINE_SRC}
    ${PROJ_SRC}
)

set(SHADER_DIR ${CMAKE_SOURCE_DIR}/assets/shaders)
file(GLOB SHADERS "${SHADER_DIR}/*.vert" "${SHADER_DIR}/*.frag")

set(SPIRV_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/assets/shaders)
foreach (SHADER ${SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SPIRV_OUTPUT_FILE ${SPIRV_OUTPUT_DIR}/${SHADER_NAME}.spv)

    add_custom_command(
            OUTPUT ${SPIRV_OUTPUT_FILE}
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} -o ${SPIRV_OUTPUT_FILE} ${SHADER}
            DEPENDS ${SHADER}
            COMMENT "Compiling shader ${SHADER} to SPIR-V"
    )

    list(APPEND SPIRV_OUTPUTS ${SPIRV_OUTPUT_FILE})
endforeach ()

add_custom_target(compile_shaders DEPENDS ${SPIRV_OUTPUTS})
add_dependencies(${PROJ_NAME} compile_shaders)

set(LIBS pthread dl)
set(GRAPHICS_LIBS SDL3::SDL3 Vulkan::Vulkan glm)
set(LOGGING_LIBS spdlog::spdlog fmt)
set(IMAGE_LIBS)
set(AUDIO_LIBS openal)

target_link_libraries(${PROJ_NAME} PRIVATE ${LIBS} ${GRAPHICS_LIBS} ${LOGGING_LIBS} ${IMAGE_LIBS} ${AUDIO_LIBS})
