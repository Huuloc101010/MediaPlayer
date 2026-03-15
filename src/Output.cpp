#include "Output.h"


Output::~Output()
{
    m_QueueSafe.release();
}

bool Output::StartThread()
{
    m_ThreadShow = std::jthread(&Output::ThreadProcessFramePtr, this);
    return true;
}

void Output::Stop()
{
    ControlFunction::Stop();
    m_QueueSafe.release();
}

void Output::Exit()
{
    if(m_ThreadShow.joinable())
    {
        m_ThreadShow.join();
    }
}

void Output::push_queue(UniqueFramePtr FramePtr)
{
    m_QueueSafe.push(std::move(FramePtr));
}

void Output::SetLimitQueueOutput(const int LimitValue)
{
    m_QueueSafe.SetLimitQueue(LimitValue);
}

void Output::clear()
{
    LOGW("Not Implement");
}

double Output::get_clock()
{
    LOGW("Not Implement");
    return {};
}
