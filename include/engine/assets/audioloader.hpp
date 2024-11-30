#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace muon {

    struct OggProperties {
        int32_t channels;
        int64_t sample_rate;
    };

    void loadOggFile(const std::string &path, std::vector<short> &audio_data, OggProperties &properties);

}
