#include "engine/vulkan/font.hpp"

#include <memory>
#include <spdlog/spdlog.h>
#include <sys/types.h>

#include "AtlasGenerator.h"
#include "BitmapAtlasStorage.h"
#include "ImmediateAtlasGenerator.h"
#include "TightAtlasPacker.h"
#include "Workload.h"
#include "core/BitmapRef.hpp"
#include "glyph-generators.h"

#include "engine/vulkan/texture.hpp"
#include "utils.hpp"

template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
std::shared_ptr<Texture> create_atlas(Device &device, std::vector<msdf_atlas::GlyphGeometry> glyphs, uint32_t width, uint32_t height) {
    msdf_atlas::GeneratorAttributes attributes;
    attributes.config.overlapSupport = true;
    attributes.scanlinePass = true;

    msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
    generator.setAttributes(attributes);
    generator.setThreadCount(3);
    generator.generate(glyphs.data(), glyphs.size());

    msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

    TextureCreateInfo info{};
    info.image_format = VK_FORMAT_R8G8B8_SRGB;
    info.instance_size = 3;
    info.width = width;
    info.height = height;
    info.image_data = (void *)bitmap.pixels;

    std::shared_ptr texture = std::make_shared<Texture>(device, info);
    return texture;
}

Font::Font(std::string &font_path, Device &device) {
    msdfgen::FreetypeHandle *freetype = msdfgen::initializeFreetype();
    msdfgen::FontHandle *font = msdfgen::loadFont(freetype, font_path.c_str());
    if (font == nullptr) {
        spdlog::error("Failed to load font: {}", font_path);
        exit(exitcode::FAILURE);
    }

    struct CharsetRange {
        uint32_t begin;
        uint32_t end;
    };

    static const CharsetRange charset_ranges[] = {
        { 0x0020, 0x00FF }
    };

    msdf_atlas::Charset charset;
    for (auto range : charset_ranges) {
        for (uint32_t c = range.begin; c <= range.end; c++) {
            charset.add(c);
        }
    }

    double font_scale = 1.0;
    font_geometry = msdf_atlas::FontGeometry(&glyphs);
    int glyphs_loaded = font_geometry.loadCharset(font, font_scale, charset);
    spdlog::debug("Loaded {} glyphs from font (out of {})", glyphs_loaded, charset.size());

    double em_size = 40.0;

    msdf_atlas::TightAtlasPacker atlas_packer;
    atlas_packer.setPixelRange(2.0);
    atlas_packer.setMiterLimit(1.0);
    atlas_packer.setScale(em_size);
    int remaining = atlas_packer.pack(glyphs.data(), glyphs.size());

    int width;
    int height;
    atlas_packer.getDimensions(width, height);
    em_size = atlas_packer.getScale();

    #define DEFAULT_ANGLE_THRESHOLD 3.0
    #define LCG_MULTIPLIER 6364136223846793005ull
    #define LCG_INCREMENT 1442695040888963407ull
    #define THREAD_COUNT 3

    uint64_t colouring_seed = 0;
    bool expensive_colouring = false;
    if (expensive_colouring) {
        msdf_atlas::Workload([&glyphs = glyphs, &colouring_seed](int i, int thread_no) -> bool {
            unsigned long long glyph_seed = (LCG_MULTIPLIER * (colouring_seed ^ i) + LCG_INCREMENT) * !!colouring_seed;
            glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyph_seed);
            return true;
        }, glyphs.size()).finish(THREAD_COUNT);
    } else {
        unsigned long long glyph_seed = colouring_seed;
        for (msdf_atlas::GlyphGeometry &glyph : glyphs) {
            glyph_seed *= LCG_MULTIPLIER;
            glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyph_seed);
        }
    }

    atlas = create_atlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>(device, glyphs, width, height);

    // glyphs[0].getQuadAtlasBounds(double &l, double &b, double &r, double &t);

    msdfgen::destroyFont(font);
    msdfgen::deinitializeFreetype(freetype);
}
