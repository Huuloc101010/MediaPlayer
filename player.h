#ifndef _PLAYER_H_
#define _PLAYER_H_
extern "C"
{
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS
 
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
class player
{
public:
    player() = default;
    ~player() = default;
     // AVFormatContext đại diện cho container mp4
 // AVStream là đại diện cho mỗi luồng trong container ex: pFormatCtx->streams[i]
 
 inline const char* err2str(int errnum);
 inline const char* ts2timestr(int64_t ts, AVRational tb);

 int output_video_frame(AVFrame *frame);

 
 int output_audio_frame(AVFrame *frame);
 
 int decode_packet(AVCodecContext *dec, const AVPacket *pkt);
 
 int open_codec_context(int *stream_idx,
                              AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);
 
 int get_format_from_sample_fmt(const char **fmt,
                                      enum AVSampleFormat sample_fmt);
 
int run (int argc, char **argv);
private:
     AVFormatContext *fmt_ctx = NULL;
     AVCodecContext *video_dec_ctx = NULL, *audio_dec_ctx;
     int width, height;
     enum AVPixelFormat pix_fmt;
     AVStream *video_stream = NULL, *audio_stream = NULL;
     const char *src_filename = NULL;
     const char *video_dst_filename = NULL;
     const char *audio_dst_filename = NULL;
     FILE *video_dst_file = NULL;
     FILE *audio_dst_file = NULL;
    
     uint8_t *video_dst_data[4] = {NULL};
     int      video_dst_linesize[4];
     int video_dst_bufsize;
    
     int video_stream_idx = -1, audio_stream_idx = -1;
     AVFrame *frame = NULL;
     AVPacket *pkt = NULL;
     int video_frame_count = 0;
     int audio_frame_count = 0;
};

#endif /*_PLAYER_H_*/