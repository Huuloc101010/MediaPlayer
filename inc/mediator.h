#ifndef _MEDIATOR_
#define _MEDIATOR_

class mediator
{
public:
    virtual double GetAudioClock() = 0;
    virtual AVRational GetTimeBaseAudio() = 0;
    virtual AVRational GetTimeBaseVideo() = 0;
};

#endif /* _MEDIATOR_ */
