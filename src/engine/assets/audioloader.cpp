#include <cstdio>
#include <engine/assets/audioloader.hpp>

#include <spdlog/spdlog.h>

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

namespace muon {

    void loadOggFile(const std::string &path, std::vector<short> &audio_data, OggProperties &properties) {
        stb_vorbis *vorbis = stb_vorbis_open_filename(path.c_str(), nullptr, nullptr);
        if (vorbis == nullptr) {
            stb_vorbis_close(vorbis);
            spdlog::warn("Failed to load file: {}", path);
            return;
        }

        stb_vorbis_info info = stb_vorbis_get_info(vorbis);
        int sample_rate = info.sample_rate;
        int channels = info.channels;

        int sample_count = stb_vorbis_stream_length_in_samples(vorbis) * channels;
        audio_data.resize(sample_count);

        int samples_decoded = stb_vorbis_get_samples_short_interleaved(vorbis, channels, audio_data.data(), sample_count);
        audio_data.resize(samples_decoded * info.channels);

        properties.channels = channels;
        properties.sample_rate = sample_rate;

        stb_vorbis_close(vorbis);
    }

}
