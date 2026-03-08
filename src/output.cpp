#include "output.h"


output::~output()
{
    m_QueueSafe.release();
}

bool output::StartThread()
{
    m_ThreadShow = std::jthread(&output::ThreadProcessFramePtr, this);
    return true;
}

void output::Exit()
{
    if(m_ThreadShow.joinable())
    {
        m_ThreadShow.join();
    }
}

void output::push_queue(UniqueFramePtr FramePtr)
{
    m_QueueSafe.push(std::move(FramePtr));
}

void output::SetLimitQueueOutput(const int LimitValue)
{
    m_QueueSafe.SetLimitQueue(LimitValue);
}

void output::clear()
{
    LOGW("Not Implement");
}

const double output::get_clock()
{
    LOGW("Not Implement");
    return {};
}
