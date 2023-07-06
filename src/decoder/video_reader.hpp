#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <inttypes.h>
}

struct VideoReaderState {
    // Public things for other parts of the program to read from
    int width, height;
    AVRational time_base;

    // Private internal state
    AVFormatContext* av_format_ctx;
    AVCodecContext* av_codec_ctx;
    int video_stream_index;
    AVFrame* av_frame;
    AVPacket* av_packet;
    SwsContext* sws_scaler_ctx;
};
class VideoReader {
public:
    explicit VideoReader(const char* filename);
    VideoReaderState videoReaderState{};
    uint8_t* frame_buffer;
    int64_t* pts{};
    bool video_reader_open(const char* filename);
    bool video_reader_read_frame();
    bool video_reader_seek_frame(int64_t ts);
    void video_reader_close();
private:
    ~VideoReader();
};
