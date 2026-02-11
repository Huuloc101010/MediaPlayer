#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <vector>
#include <memory>
#include <mutex>
#include "define.h"

extern "C"
{
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS
 
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

class videooutput;
class audiooutput;

class player
{
public:
    player() = default;
    ~player();
 
    int run(int argc, char **argv);

private:
    std::string err2str(int errnum);
    std::string ts2timestr(int64_t ts, AVRational tb);
    int output_video_frame(AVFrame *m_frame);
    int output_audio_frame(AVFrame *m_frame);
    int decode_packet(AVCodecContext *dec, const AVPacket *pkt);
    int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *m_fmt_ctx, enum AVMediaType type);
    int get_format_from_sample_fmt(const char **fmt, enum AVSampleFormat sample_fmt);
    bool config_audio_output(AVFrame* m_frame);

    std::unique_ptr<videooutput>    m_videooutput = nullptr;
    std::unique_ptr<audiooutput>    m_audiooutput = nullptr;
    std::once_flag                  m_once_flag;
    SwrContext*                     m_swr = nullptr;
    AVFormatContext *               m_fmt_ctx = nullptr;
    AVCodecContext *                m_video_dec_ctx = nullptr, *m_audio_dec_ctx;
    int                             m_width, m_height;
    enum AVPixelFormat              m_pix_fmt;
    AVStream *                      m_video_stream = nullptr, *m_audio_stream = nullptr;
    const char *                    m_src_filename = nullptr;

    uint8_t *                       m_video_dst_data[4] = {nullptr};
    int                             m_video_dst_linesize[4];
    int                             m_video_dst_bufsize;

    int                             m_video_stream_idx = -1;
    int                             m_audio_stream_idx = -1;
    AVFrame *                       m_frame = nullptr;
    AVPacket *                      m_pkt = nullptr;
};

#endif /*_PLAYER_H_*/