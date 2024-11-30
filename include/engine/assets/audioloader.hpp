#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace muon {

    struct OggProperties {
        int32_t channels;
        int64_t sample_rate;
        int64_t sample_count;
    };

    void loadOggFile(std::string &path, std::vector<char> &audio_data, OggProperties &properties);

}
