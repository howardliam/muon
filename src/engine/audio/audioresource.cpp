#include "audioresource.hpp"
#include <AL/al.h>

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

AudioResource::AudioResource(std::string &filename) {
    load_ogg(filename);

    if (channels == 1) {
        format = AL_FORMAT_MONO16;
    } else if (channels == 2) {
        format = AL_FORMAT_STEREO16;
    }

    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, data.data(), data.size() * sizeof(short), sample_rate);

    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);
}

AudioResource::~AudioResource() {
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
}

void AudioResource::play() {
    alSourcePlay(source);
}

void AudioResource::pause() {
    alSourcePause(source);
}

void AudioResource::resume() {
    play();
}

void AudioResource::stop() {
    alSourceStop(source);
}

void AudioResource::load_ogg(std::string &filename) {
    stb_vorbis *vorbis = stb_vorbis_open_filename(filename.c_str(), nullptr, nullptr);
    if (vorbis == nullptr) {
        stb_vorbis_close(vorbis);
        return;
    }

    stb_vorbis_info info = stb_vorbis_get_info(vorbis);
    sample_rate = info.sample_rate;
    channels = info.channels;

    int sample_count = stb_vorbis_stream_length_in_samples(vorbis) * channels;
    std::vector<short> output(sample_count);
    int samples_decoded = stb_vorbis_get_samples_short_interleaved(vorbis, channels, output.data(), sample_count);

    stb_vorbis_close(vorbis);

    output.resize(samples_decoded * info.channels);

    data = output;
    loaded = true;
}
