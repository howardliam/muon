#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "engine/vulkan/model.hpp"
#include "engine/vulkan/texture.hpp"

namespace muon {

    struct TransformComponent {
       glm::mat4 transform;
    };

    struct ModelComponent {
        std::weak_ptr<Model> model;
    };

    struct TextureComponent {
        std::weak_ptr<Texture> texture;
    };

}
