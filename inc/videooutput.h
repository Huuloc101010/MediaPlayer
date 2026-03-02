#ifndef _VIDEO_OUTPUT_
#define _VIDEO_OUTPUT_

#include <mutex>
#include <thread>
#include <shared_mutex>
#include <iostream>
#include <memory>
#include "define.h"
#include "output.h"

class mediator;

class videooutput : public output
{
public:
    videooutput(mediator* mediator);
    ~videooutput();
    bool UpdateYUVTexture(const yuv& ndata);
    bool ConvertFramePtrToRawData(UniqueFramePtr frame);
    void ThreadProcessFramePtr() override;

private:
    mediator*            m_Mediator{};
};

#endif /*_VIDEO_OUTPUT_*/