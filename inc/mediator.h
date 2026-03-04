#ifndef _MEDIATOR_
#define _MEDIATOR_

class mediator
{
public:
    virtual double GetAudioClock() = 0;
    virtual AVRational GetTimeBaseAudio() = 0;
    virtual AVRational GetTimeBaseVideo() = 0;
    virtual int output_video_frame(UniqueFramePtr frame) = 0;
    virtual int output_audio_frame(UniqueFramePtr frame) = 0;
    virtual const std::string err2str(int errnum) = 0;
    virtual std::atomic<PlayerState>& GetCurrentState() = 0;
    virtual bool ConfigVideoOutput() = 0;
    virtual bool ConfigAudioOutput() = 0;
    virtual int decode_packet(UniquePacketPtr pkt, const bool IsFlushDecoder = false) = 0;
    virtual bool InitVideoDecoder(const AVCodecID codecID, AVCodecParameters* codec_par) = 0;
    virtual bool InitAudioDecoder(const AVCodecID codecID, AVCodecParameters* codec_par) = 0;
    virtual bool InitView() = 0;
    virtual bool UpdateYUVTexture(const yuv& ndata) = 0;
    virtual void PushEvent(PlayerEvent Event) = 0;
};

#endif /* _MEDIATOR_ */
