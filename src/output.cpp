#include "output.h"


output::~output()
{
    m_Exiting = true;
    m_QueueSafe.release();
}

bool output::StartThread()
{
    m_ThreadShow = std::jthread(&output::ThreadProcessFramePtr, this);
    return true;
}

void output::push_queue(UniqueFramePtr FramePtr)
{
    m_QueueSafe.push(std::move(FramePtr));
}

void output::start()
{
    LOGW("Not Implement");
}

void output::stop()
{
    LOGW("Not Implement");
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
