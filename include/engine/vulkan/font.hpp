#pragma once

#include <string>
#include <vector>
#include <memory>

#define MSDFGEN_PUBLIC
#include "FontGeometry.h"
#include "GlyphGeometry.h"

#include "engine/vulkan/device.hpp"
#include "engine/vulkan/texture.hpp"

namespace muon {

    class Font {
    public:
        Font(std::string &font_path, Device &device);
        ~Font() = default;

        std::vector<msdf_atlas::GlyphGeometry> getGlyphs() { return glyphs; }
        msdf_atlas::FontGeometry getFontGeometry() { return font_geometry; }
        std::shared_ptr<Texture> getAtlas() const { return atlas; }

    private:
        std::vector<msdf_atlas::GlyphGeometry> glyphs{};
        msdf_atlas::FontGeometry font_geometry;
        std::shared_ptr<Texture> atlas;
    };

}
