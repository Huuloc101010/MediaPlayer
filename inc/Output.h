#ifndef __OUTPUT_
#define __OUTPUT_

#include <thread>
#include <SafeQueue.h>
#include "Define.h"
#include "ControlFunction.h"

class Output : public ControlFunction
{
public:
    Output() = default;
    virtual ~Output();
    virtual void PushQueue(UniqueFramePtr FramePtr) final;
    virtual void SetLimitQueueOutput(const int LimitValue);
    virtual void Clear();
    virtual double GetClock();
    virtual bool StartThread();
    virtual void Stop() override;
    virtual void Exit() override;
    void FlushData() override;
protected:
    virtual void ThreadProcessFramePtr() = 0;

    Clock                        m_Clock{};
    SafeQueue<UniqueFramePtr>    m_QueueSafe{};
    std::jthread                 m_ThreadShow;
};

#endif /* _VIDEO_OUTPUT_ */