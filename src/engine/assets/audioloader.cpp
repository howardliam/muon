#include <engine/assets/audioloader.hpp>

#include <fstream>
#include <spdlog/spdlog.h>
#include <vorbis/vorbisfile.h>

namespace muon {

    size_t read(void *buffer, size_t size, size_t element_count, void *data_source) {
        auto &stream = *static_cast<std::ifstream *>(data_source);
        stream.read(static_cast<char *>(buffer), size * element_count);
        return stream.gcount();
    }

    int seek(void *data_source, ogg_int64_t offset, int origin) {
        static const std::vector<std::ios_base::seekdir> seek_directions{
            std::ios_base::beg, std::ios_base::cur, std::ios_base::end
        };

        auto &stream = *static_cast<std::ifstream *>(data_source);
        stream.seekg(offset, seek_directions.at(origin));
        stream.clear();
        return 0;
    }

    int64_t tell(void *data_source) {
        auto &stream = *static_cast<std::ifstream *>(data_source);
        const auto position = stream.tellg();
        return static_cast<long>(position);
    }

    void loadOggFile(std::string &path, std::vector<char> &audio_data, OggProperties &properties) {
        std::ifstream stream{path, std::ios::binary};
        if (!stream) {
            spdlog::error("Failed to open file: {}", path);
            return;
        }

        OggVorbis_File file;
        ov_callbacks callbacks = {read, seek, nullptr, tell};
        int res = ov_open_callbacks(&stream, &file, nullptr, 0, callbacks);
        if (res < 0) {
            ov_clear(&file);
            spdlog::error("Error opening file: {}", res);
            return;
        }

        vorbis_info *info = ov_info(&file, -1);
        properties.channels = info->channels;
        properties.sample_rate = info->rate;

        int64_t total_samples = static_cast<int64_t>(ov_pcm_total(&file, -1));

        audio_data.resize(total_samples * properties.channels);

        audio_data.resize(4096);
        int current_section = 0;

        bool eof = false;
        while (!eof) {
            long ret = ov_read(&file, audio_data.data(), audio_data.size(), 0, 2, 1, &current_section);
            if (ret == 0) {
                eof = true;
            } else if (ret < 1) {
                if(ret==OV_EBADLINK){
                    ov_clear(&file);
                    spdlog::error("Corrupted bitstream");
                    return;
                }
            }
        }

        ov_clear(&file);
    }

}
