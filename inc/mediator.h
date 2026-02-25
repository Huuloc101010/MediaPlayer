#ifndef _MEDIATOR_
#define _MEDIATOR_

class mediator
{
public:
    virtual double GetAudioClock() = 0;
    virtual AVRational GetTimeBaseAudio() = 0;
    virtual AVRational GetTimeBaseVideo() = 0;
    virtual int output_video_frame() = 0;
    virtual int output_audio_frame() = 0;
};

#endif /* _MEDIATOR_ */
