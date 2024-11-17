cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)

set(PROJ_NAME vulkan-sdl3-starter)

project(${PROJ_NAME} VERSION 0.1.0 LANGUAGES C CXX)

find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)
find_package(glm REQUIRED)
find_package(spdlog REQUIRED)

set(ENGINE_SRC
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

set(STB_SRC
    src/stb/stb_image.c
    src/stb/stb_image.h
)

set(TOJ_SRC
    src/tinyobjloader/tiny_obj_loader.cpp
    src/tinyobjloader/tiny_obj_loader.h
)

add_executable(${PROJ_NAME}
    ${ENGINE_SRC}
    ${PROJ_SRC}
    ${STB_SRC}
    ${TOJ_SRC}
)

set(SHADER_DIR ${CMAKE_SOURCE_DIR}/assets/shaders)
file(GLOB SHADERS "${SHADER_DIR}/*.vert" "${SHADER_DIR}/*.frag")
message(STATUS "Found shader files: ${SHADERS}")

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

set(LIBS
    pthread
    dl
    SDL3::SDL3
    Vulkan::Vulkan
    glm
    spdlog::spdlog
    fmt
)

target_link_libraries(${PROJ_NAME} ${LIBS})
