#include "output.h"

output::output() : m_ThreadShow(&output::ThreadProcessFramePtr, this) {}

output::~output()
{
    m_exiting = true;
    if(m_ThreadShow.joinable())
    {
        m_ThreadShow.join();
    }
}

void output::push_queue(UniqueFramePtr FramePtr)
{
    std::lock_guard<std::mutex> lock_guard(m_QueueSafe.mutex);
    m_QueueSafe.queue.emplace_front(std::move(FramePtr));
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
