#ifndef _DEMUXER_
#define _DEMUXER_

#include "Define.h"
#include "ControlFunction.h"

class Mediator;

class Demuxer : public ControlFunction
{
public:
    Demuxer(Mediator* Mediator);
    ~Demuxer() = default;
    int StartPlay(const std::string& Mediafile);
    void ThreadReadFrame();
    int OpenCodecContext(int *stream_idx, AVFormatContext *fmt_ctx, enum AVMediaType type);
    AVRational GetTimeBaseVideo();
    AVRational GetTimeBaseAudio();
    int GetVideoStreamIndex();
    int GetAudioStreamIndex();
    double GetTotalVideoTime();
    void Exit() override;

private:
    Mediator*              m_Mediator{};
    std::jthread           m_ThreadReadFrame;
    UniqueFormatContext    m_FormatContext = nullptr;
    AVStream*              m_VideoStream = nullptr;
    AVStream*              m_AudioStream = nullptr;
    int                    m_Width{}, m_Height{};
    int                    m_VideoStreamIndex = -1;
    int                    m_AudioStreamIndex = -1;
};

#endif /* _DEMUXER_ */