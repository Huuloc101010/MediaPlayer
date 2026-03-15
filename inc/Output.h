#ifndef __OUTPUT_
#define __OUTPUT_

#include <thread>
#include <SafeQueue.h>
#include "Define.h"
#include "ControlFunction.h"

class output : public controlfunction
{
public:
    output() = default;
    virtual ~output();
    virtual void push_queue(UniqueFramePtr FramePtr) final;
    virtual void SetLimitQueueOutput(const int LimitValue);
    virtual void clear();
    virtual double get_clock();
    virtual bool StartThread();
    virtual void Stop() override;
    virtual void Exit() override;
protected:
    virtual void ThreadProcessFramePtr() = 0;

    Clock                        m_Clock{};
    queue_safe<UniqueFramePtr>   m_QueueSafe{};
    std::jthread                 m_ThreadShow;
};

#endif /* _VIDEO_OUTPUT_ */