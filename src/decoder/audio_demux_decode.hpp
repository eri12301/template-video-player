#pragma once

extern "C"
{
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <string>

class AudioDecoder
{
private:
    std::string m_format;
    int m_sample_rate;
    int m_channels;
public:
    int demuxDecode(const char* src_filepath, const char* audio_dst_filepath);
    const std::string& getFormat() const {
        return m_format;
    }
    int getSampleRate() const {
        return m_sample_rate;
    }
    int getNumChannels() const {
        return m_channels;
    }
};