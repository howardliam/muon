cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 20)

set(PROJ_NAME muon)

project(${PROJ_NAME} VERSION 0.1.0 LANGUAGES C CXX)

find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)
find_package(glm REQUIRED)
find_package(spdlog REQUIRED)

find_package(OpenAL REQUIRED)

set(MSDF_ATLAS_BUILD_STANDALONE OFF CACHE BOOL "" FORCE)
set(MSDF_ATLAS_USE_VCPKG OFF CACHE BOOL "" FORCE)
set(MSDF_ATLAS_USE_SKIA OFF CACHE BOOL "" FORCE)
add_subdirectory(external/msdf-atlas-gen)

add_subdirectory(external/assimp)

include_directories(
    libs/stb
    external/entt/single_include/entt
    external/tomlplusplus/include
)

set(ENGINE_SRC
    # Render
    src/engine/render/common.cpp
    src/engine/render/rendermodule.cpp
    src/engine/render/scenemodule.cpp

    # Rendering systems
    src/engine/rendering/rendersystem.cpp
    src/engine/rendering/textrenderer.cpp

    # Vulkan
    src/engine/vulkan/buffer.cpp
    src/engine/vulkan/descriptors.cpp
    src/engine/vulkan/device.cpp
    src/engine/vulkan/font.cpp
    src/engine/vulkan/model.cpp
    src/engine/vulkan/pipeline.cpp
    src/engine/vulkan/renderer.cpp
    src/engine/vulkan/swapchain.cpp
    src/engine/vulkan/texture.cpp

    # Window
    src/engine/window/window.cpp
)

set(OTHER_SRC
    # Audio

    # Input
    src/input/inputmanager.cpp
    src/input/keyboard.cpp
    src/input/mouse.cpp

    # Scene
    src/scene/camera.cpp
    src/scene/scene.cpp
)

set(PROJ_SRC
    src/main.cpp

    src/app.cpp
    src/app.hpp
)

add_executable(${PROJ_NAME}
    ${ENGINE_SRC}
    ${OTHER_SRC}
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
set(TEXT_LIBS msdf-atlas-gen)
set(AUDIO_LIBS openal)
set(MODEL_LIBS assimp)

set(MSDF_DIRS external/msdf-atlas-gen/msdf-atlas-gen/ external/msdf-atlas-gen/msdfgen/)

target_include_directories(${PROJ_NAME} PRIVATE include/ ${MSDF_DIRS})
target_link_libraries(${PROJ_NAME} PRIVATE ${LIBS} ${GRAPHICS_LIBS} ${LOGGING_LIBS} ${TEXT_LIBS} ${AUDIO_LIBS} ${MODEL_LIBS})
