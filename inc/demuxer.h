#ifndef _DEMUXER_
#define _DEMUXER_

#include "define.h"

class mediator;

class demuxer
{
public:
    demuxer(mediator* mediator);
    ~demuxer() = default;
    int Play(int argc, char **argv);
    void loop_read_frame();
    int open_codec_context(int *stream_idx, AVFormatContext *fmt_ctx, enum AVMediaType type);
    AVRational GetTimeBaseVideo();
    AVRational GetTimeBaseAudio();
    const int GetVideoStreamIndex();
    const int GetAudioStreamIndex();

private:
    mediator*              m_Mediator{};
    UniqueFormatContext    m_FormatContext = nullptr;
    AVStream*              m_VideoStream = nullptr;
    AVStream*              m_AudioStream = nullptr;
    int                    m_Width{}, m_Height{};
    int                    m_VideoStreamIndex = -1;
    int                    m_AudioStreamIndex = -1;
};

#endif /* _DEMUXER_ */