#include "Output.h"


Output::~Output()
{
    m_QueueSafe.Release();
}

bool Output::StartThread()
{
    m_ThreadShow = std::jthread(&Output::ThreadProcessFramePtr, this);
    return true;
}

void Output::Stop()
{
    ControlFunction::Stop();
    m_QueueSafe.Release();
}

void Output::Exit()
{
    if(m_ThreadShow.joinable())
    {
        m_ThreadShow.join();
    }
}

void Output::PushQueue(UniqueFramePtr FramePtr)
{
    m_QueueSafe.Push(std::move(FramePtr));
}

void Output::SetLimitQueueOutput(const int LimitValue)
{
    m_QueueSafe.SetLimitQueue(LimitValue);
}

void Output::Clear()
{
    LOGW("Not Implement");
}

double Output::GetClock()
{
    LOGW("Not Implement");
    return {};
}

void Output::FlushData()
{
    m_QueueSafe.Clear();
}