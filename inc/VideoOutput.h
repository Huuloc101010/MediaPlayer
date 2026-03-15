#ifndef _VIDEO_OUTPUT_
#define _VIDEO_OUTPUT_

#include <mutex>
#include <thread>
#include <shared_mutex>
#include <iostream>
#include <memory>
#include "Define.h"
#include "Output.h"
#include "ControlFunction.h"

class Mediator;

class VideoOutput : public Output
{
public:
    VideoOutput(Mediator* Mediator);
    ~VideoOutput();
    bool UpdateYUVTexture(const yuv& ndata);
    bool ConvertFramePtrToRawData(UniqueFramePtr frame);
    void ThreadProcessFramePtr() override;
    void Play() override;
    void Pause() override;
    void Stop() override;
    void Exit() override;
private:
    Mediator*            m_Mediator{};
};

#endif /*_VIDEO_OUTPUT_*/