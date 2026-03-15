#ifndef _AUDIO_OUTPUT_
#define _AUDIO_OUTPUT_

#include <SDL2/SDL.h>
#include <deque>
#include <mutex>
#include <cstdint>
#include <thread>
#include "Define.h"
#include "Output.h"
#include "ControlFunction.h"

class mediator;

class audiooutput : public output
{
public:
    audiooutput(mediator* mediator);
    ~audiooutput();

    double get_clock() override;
    void audio_convert(UniqueFramePtr FramePtr);
private:
    bool config_audio_output(UniqueFramePtr& frame);
    void ThreadProcessFramePtr() override;

    SwrContext*         m_SwrContext = nullptr;
    std::once_flag      m_OnceFlag{};
    mediator*           m_Mediator{};
};

#endif /* _AUDIO_OUTPUT_*/