#pragma once

#include <unordered_map>
#include <string>
#include <memory>

#include "engine/vulkan/model.hpp"
#include "engine/vulkan/texture.hpp"

namespace muon {
    class AssetStore {
    public:

    private:
        std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
        std::unordered_map<std::string, std::shared_ptr<Model>> models;

    };
}
