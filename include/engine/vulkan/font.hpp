#pragma once

#include <string>

#define MSDFGEN_PUBLIC
#include "FontGeometry.h"
#include "GlyphGeometry.h"

#include "engine/vulkan/device.hpp"
#include "engine/vulkan/texture.hpp"

class Font {
public:
    Font(std::string &font_path, Device &device);
    ~Font() = default;

    std::shared_ptr<Texture> get_atlas() { return atlas; }

private:
    std::vector<msdf_atlas::GlyphGeometry> glyphs{};
    msdf_atlas::FontGeometry font_geometry;

    std::shared_ptr<Texture> atlas;
};
