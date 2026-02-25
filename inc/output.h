#ifndef __OUTPUT_
#define __OUTPUT_

#include <thread>
#include <queue_safe.h>
#include "define.h"

class output
{
public:
    output();
    virtual ~output();
    virtual void push_queue(UniqueFramePtr FramePtr) final;
    virtual void start();
    virtual void stop();
    virtual void clear();
    virtual const double get_clock();
protected:
    virtual void ThreadProcessFramePtr() = 0;

    Clock                        m_Clock{};
    std::atomic<bool>            m_Exiting = false;
    queue_safe<UniqueFramePtr>   m_QueueSafe{};
    std::jthread                 m_ThreadShow;
};

#endif /* _VIDEO_OUTPUT_ */