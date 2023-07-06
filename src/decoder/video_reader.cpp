#include <stdexcept>
#include "video_reader.hpp"

// av_err2str returns a temporary array. This doesn't work in gcc.
// This function can be used as a replacement for av_err2str.
static const char* av_make_error(int errnum) {
    static char str[AV_ERROR_MAX_STRING_SIZE];
    memset(str, 0, sizeof(str));
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}

static AVPixelFormat correct_for_deprecated_pixel_format(AVPixelFormat pix_fmt) {
    // Fix swscaler deprecated pixel format warning
    // (YUVJ has been deprecated, change pixel format to regular YUV)
    switch (pix_fmt) {
        case AV_PIX_FMT_YUVJ420P: return AV_PIX_FMT_YUV420P;
        case AV_PIX_FMT_YUVJ422P: return AV_PIX_FMT_YUV422P;
        case AV_PIX_FMT_YUVJ444P: return AV_PIX_FMT_YUV444P;
        case AV_PIX_FMT_YUVJ440P: return AV_PIX_FMT_YUV440P;
        default:                  return pix_fmt;
    }
}

bool VideoReader::video_reader_read_frame() {

    // Unpack members of state
    auto& width = videoReaderState.width;
    auto& height = videoReaderState.height;
    auto& av_format_ctx = videoReaderState.av_format_ctx;
    auto& av_codec_ctx = videoReaderState.av_codec_ctx;
    auto& video_stream_index = videoReaderState.video_stream_index;
    auto& av_frame = videoReaderState.av_frame;
    auto& av_packet = videoReaderState.av_packet;
    auto& sws_scaler_ctx = videoReaderState.sws_scaler_ctx;

    // Decode one frame
    int response;
    while (av_read_frame(av_format_ctx, av_packet) >= 0) {
        if (av_packet->stream_index != video_stream_index) {
            av_packet_unref(av_packet);
            continue;
        }

        response = avcodec_send_packet(av_codec_ctx, av_packet);
        if (response < 0) {
            printf("Failed to decode packet: %s\n", av_make_error(response));
            return false;
        }

        response = avcodec_receive_frame(av_codec_ctx, av_frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            av_packet_unref(av_packet);
            continue;
        } else if (response < 0) {
            printf("Failed to decode packet: %s\n", av_make_error(response));
            return false;
        }

        av_packet_unref(av_packet);
        break;
    }

    *pts = av_frame->pts;

    // Set up sws scaler
    if (!sws_scaler_ctx) {
        auto source_pix_fmt = correct_for_deprecated_pixel_format(av_codec_ctx->pix_fmt);
        sws_scaler_ctx = sws_getContext(width, height, source_pix_fmt,
                                        width, height, AV_PIX_FMT_RGB0,
                                        SWS_BILINEAR, nullptr, nullptr, nullptr);
    }
    if (!sws_scaler_ctx) {
        printf("Couldn't initialize sw scaler\n");
        return false;
    }

    uint8_t* dest[4] = { frame_buffer, nullptr, nullptr, nullptr };
    int dest_linesize[4] = { width * 4, 0, 0, 0 };
    sws_scale(sws_scaler_ctx, av_frame->data, av_frame->linesize, 0, av_frame->height, dest, dest_linesize);

    return true;
}

bool VideoReader::video_reader_seek_frame(int64_t ts) {

    // Unpack members of state
    auto& av_format_ctx = videoReaderState.av_format_ctx;
    auto& av_codec_ctx = videoReaderState.av_codec_ctx;
    auto& video_stream_index = videoReaderState.video_stream_index;
    auto& av_packet = videoReaderState.av_packet;
    auto& av_frame = videoReaderState.av_frame;

    av_seek_frame(av_format_ctx, video_stream_index, ts, AVSEEK_FLAG_BACKWARD);

    // av_seek_frame takes effect after one frame, so I'm decoding one here
    // so that the next call to video_reader_read_frame() will give the correct
    // frame
    int response;
    while (av_read_frame(av_format_ctx, av_packet) >= 0) {
        if (av_packet->stream_index != video_stream_index) {
            av_packet_unref(av_packet);
            continue;
        }

        response = avcodec_send_packet(av_codec_ctx, av_packet);
        if (response < 0) {
            printf("Failed to decode packet: %s\n", av_make_error(response));
            return false;
        }

        response = avcodec_receive_frame(av_codec_ctx, av_frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            av_packet_unref(av_packet);
            continue;
        } else if (response < 0) {
            printf("Failed to decode packet: %s\n", av_make_error(response));
            return false;
        }

        av_packet_unref(av_packet);
        break;
    }
    *pts = ts;
    return true;
}

void VideoReader::video_reader_close() {
    sws_freeContext(videoReaderState.sws_scaler_ctx);
    avformat_close_input(&videoReaderState.av_format_ctx);
    avformat_free_context(videoReaderState.av_format_ctx);
    av_frame_free(&videoReaderState.av_frame);
    av_packet_free(&videoReaderState.av_packet);
    avcodec_free_context(&videoReaderState.av_codec_ctx);
}

bool VideoReader::video_reader_open(const char *filename) {
    // Unpack members of state
    auto& width = videoReaderState.width;
    auto& height = videoReaderState.height;
    auto& time_base = videoReaderState.time_base;
    auto& av_format_ctx = videoReaderState.av_format_ctx;
    auto& av_codec_ctx = videoReaderState.av_codec_ctx;
    auto& video_stream_index = videoReaderState.video_stream_index;
    auto& av_frame = videoReaderState.av_frame;
    auto& av_packet = videoReaderState.av_packet;

    // Open the file using libavformat
    av_format_ctx = avformat_alloc_context();
    if (!av_format_ctx) {
        printf("Couldn't created AVFormatContext\n");
        return false;
    }

    if (avformat_open_input(&av_format_ctx, filename, nullptr, nullptr) != 0) {
        printf("Couldn't open video file\n");
        return false;
    }

    // Find the first valid video stream inside the file
    video_stream_index = -1;
    AVCodecParameters* av_codec_params;
    const AVCodec* av_codec;
    for (int i = 0; i < av_format_ctx->nb_streams; ++i) {
        av_codec_params = av_format_ctx->streams[i]->codecpar;
        av_codec = avcodec_find_decoder(av_codec_params->codec_id);
        if (!av_codec) {
            continue;
        }
        if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            width = av_codec_params->width;
            height = av_codec_params->height;
            time_base = av_format_ctx->streams[i]->time_base;
            break;
        }
    }
    if (video_stream_index == -1) {
        printf("Couldn't find valid video stream inside file\n");
        return false;
    }

    // Set up a codec context for the decoder
    av_codec_ctx = avcodec_alloc_context3(av_codec);
    if (!av_codec_ctx) {
        printf("Couldn't create AVCodecContext\n");
        return false;
    }
    if (avcodec_parameters_to_context(av_codec_ctx, av_codec_params) < 0) {
        printf("Couldn't initialize AVCodecContext\n");
        return false;
    }
    if (avcodec_open2(av_codec_ctx, av_codec, nullptr) < 0) {
        printf("Couldn't open codec\n");
        return false;
    }

    av_frame = av_frame_alloc();
    if (!av_frame) {
        printf("Couldn't allocate AVFrame\n");
        return false;
    }
    av_packet = av_packet_alloc();
    if (!av_packet) {
        printf("Couldn't allocate AVPacket\n");
        return false;
    }

    return true;
}

VideoReader::VideoReader(const char *filename) {
    if (!this->video_reader_open(filename)) {
        throw std::invalid_argument("Couldn't open video file (make sure you set a video file that exists)");
    }

    constexpr int ALIGNMENT = 128;
    const int frame_width = this->videoReaderState.width;
    const int frame_height = this->videoReaderState.height;
    //posix_memalign replacement
    this->frame_buffer = static_cast<uint8_t *>(_aligned_malloc(frame_width * frame_height * 4, ALIGNMENT));
    if (!frame_buffer) {
        throw std::runtime_error("Couldn't allocate frame buffer");
    }
    this->pts = static_cast<int64_t *>(malloc(sizeof(int64_t)));
}

VideoReader::~VideoReader() {
    this->video_reader_close();
}
