#ifndef _MEDIATOR_
#define _MEDIATOR_

class Mediator
{
public:
    virtual double GetAudioClock() = 0;
    virtual AVRational GetTimeBaseAudio() = 0;
    virtual AVRational GetTimeBaseVideo() = 0;
    virtual int OutputVideoFrame(UniqueFramePtr frame) = 0;
    virtual int OutputAudioFrame(UniqueFramePtr frame) = 0;
    virtual std::string err2str(int errnum) = 0;
    virtual std::atomic<PlayerState>& GetCurrentState() = 0;
    virtual bool ConfigVideoOutput() = 0;
    virtual bool ConfigAudioOutput() = 0;
    virtual int DecodePacket(UniquePacketPtr pkt, const bool IsFlushDecoder = false) = 0;
    virtual bool InitVideoDecoder(const AVCodecID codecID, AVCodecParameters* codec_par) = 0;
    virtual bool InitAudioDecoder(const AVCodecID codecID, AVCodecParameters* codec_par) = 0;
    virtual bool InitView() = 0;
    virtual bool UpdateYUVTexture(const yuv& ndata) = 0;
    virtual void PushEvent(PlayerEvent Event) = 0;
    virtual void PushSDLAudioData(const uint8_t* data, size_t Size) = 0;
    virtual bool AudioConfig(int sample_rate,
                             int channels,
                             SDL_AudioFormat format,
                             int first_pts,
                             int samples = 1024) = 0;
};

#endif /* _MEDIATOR_ */
