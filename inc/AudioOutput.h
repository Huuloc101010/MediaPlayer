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

class Mediator;

class AudioOutput : public Output
{
public:
    AudioOutput(Mediator* Mediator);
    ~AudioOutput();

    double get_clock() override;
    void audio_convert(UniqueFramePtr FramePtr);
private:
    bool config_audio_output(UniqueFramePtr& frame);
    void ThreadProcessFramePtr() override;

    SwrContext*         m_SwrContext = nullptr;
    std::once_flag      m_OnceFlag{};
    Mediator*           m_Mediator{};
};

#endif /* _AUDIO_OUTPUT_*/