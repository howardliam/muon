#include "engine/assets/imageloader.hpp"

#include <csetjmp>
#include <fstream>
#include <istream>

#include <png.h>
#include <spdlog/spdlog.h>

namespace muon {

    void pngReader(png_structp png, png_bytep data, png_size_t length) {
        std::istream *stream = static_cast<std::istream *>(png_get_io_ptr(png));
        if(!stream->read(reinterpret_cast<char *>(data), length)) {
            png_error(png, "Error reading PNG file");
        }
    }

    void readPngFile(std::string &path, std::vector<uint8_t> &image_data, PngProperties &properties) {
        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!png) {
            spdlog::error("Failed to create PNG read struct");
            return;
        }

        png_infop info = png_create_info_struct(png);
        if (!info) {
            png_destroy_read_struct(&png, &info, nullptr);
            spdlog::error("Failed to create PNG info struct");
            return;
        }

        if (setjmp(png_jmpbuf(png))) {
            png_destroy_read_struct(&png, &info, nullptr);
            spdlog::error("Error during PNG read");
            return;
        }

        std::ifstream image{path, std::ios::binary};
        png_set_read_fn(png, static_cast<void *>(&image), pngReader);
        png_read_info(png, info);

        properties.width = png_get_image_width(png, info);
        properties.height = png_get_image_height(png, info);
        properties.color_type = png_get_color_type(png, info);
        properties.bit_depth = png_get_bit_depth(png, info);

        png_read_update_info(png, info);

        png_size_t row_bytes = png_get_rowbytes(png, info);
        image_data.resize(row_bytes * properties.height);
        std::vector<png_bytep> row_pointers(properties.height);

        for (png_uint_32 i = 0; i < properties.height; ++i) {
            row_pointers[i] = image_data.data() + i * row_bytes;
        }

        png_read_image(png, row_pointers.data());
        png_destroy_read_struct(&png, &info, nullptr);
    }

}
