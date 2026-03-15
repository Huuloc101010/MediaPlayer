#ifndef _DEMUXER_
#define _DEMUXER_

#include "define.h"
#include "controlfunction.h"

class mediator;

class demuxer : public controlfunction
{
public:
    demuxer(mediator* mediator);
    ~demuxer() = default;
    int StartPlay(const std::string& Mediafile);
    void loop_read_frame();
    int open_codec_context(int *stream_idx, AVFormatContext *fmt_ctx, enum AVMediaType type);
    AVRational GetTimeBaseVideo();
    AVRational GetTimeBaseAudio();
    int GetVideoStreamIndex();
    int GetAudioStreamIndex();
    void Exit() override;

private:
    mediator*              m_Mediator{};
    std::jthread           m_ThreadReadFrame;
    UniqueFormatContext    m_FormatContext = nullptr;
    AVStream*              m_VideoStream = nullptr;
    AVStream*              m_AudioStream = nullptr;
    int                    m_Width{}, m_Height{};
    int                    m_VideoStreamIndex = -1;
    int                    m_AudioStreamIndex = -1;
};

#endif /* _DEMUXER_ */