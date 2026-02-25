#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <vector>
#include <memory>
#include <mutex>
#include "define.h"
#include "mediator.h"

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

class player : public mediator
{
public:
    player() = default;
    ~player() = default;
 
    int run(int argc, char **argv);
    double GetAudioClock() override;
    AVRational GetTimeBaseAudio() override;
    AVRational GetTimeBaseVideo() override;

private:
    std::string err2str(int errnum);
    std::string ts2timestr(int64_t ts, AVRational tb);
    int output_video_frame();
    int output_audio_frame();
    int decode_packet(AVCodecContext *dec, UniquePacketPtr pkt, UniqueFramePtr& frame);
    int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *m_FormatContext, enum AVMediaType type);
    int get_format_from_sample_fmt(const char **fmt, enum AVSampleFormat sample_fmt);
    bool config_audio_output();
    void clean_resource();
    void loop_read_frame();
    int init_decoder(AVCodecID codecID, AVCodecContext **dec_ctx, AVCodecParameters* codec_par);

    std::unique_ptr<videooutput>    m_VideoOutput = nullptr;
    std::unique_ptr<audiooutput>    m_AudioOutput = nullptr;
   
    enum AVPixelFormat              m_PixelFormat;
    AVFormatContext*                m_FormatContext = nullptr;
    AVCodecContext*                 m_VideoDecodeContext = nullptr, *m_AudioDecodeContext;
    AVStream*                       m_VideoStream = nullptr, *m_AudioStream = nullptr;
    uint8_t*                        m_VideoDtsData[4] = {nullptr};
    UniqueFramePtr                  m_Frame = nullptr;
    UniquePacketPtr                 m_Packet = nullptr;
    int                             m_Width{}, m_Height{};
    int                             m_VideoDtsLineSize[4];
    int                             m_VideoDtsBuffSize{};
    int                             m_VideoStreamIndex = -1;
    int                             m_AudioStreamIndex = -1;
    std::string                     m_SourceFileName{};

};

#endif /*_PLAYER_H_*/