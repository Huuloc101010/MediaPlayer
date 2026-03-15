#ifndef _QUEUE_SAFE_
#define _QUEUE_SAFE_

#include <optional>
#include <deque>
#include <condition_variable>
#include <shared_mutex>
#include "log.h"

template<typename T>
class queue_safe
{
public:
    queue_safe() = default;

    ~queue_safe() = default;

    void push(T data)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        // Log for debug only
        //LOGE("Current Queue size [{}]", m_queue.size());
        m_condition_variable.wait(lock, [this]
        {
            return  m_Exiting
                 || m_LimitValue == 0
                 || m_queue.size() < m_LimitValue;
        });
        if(m_Exiting)
        {
            return;
        }
        m_queue.push_back(std::move(data));
        lock.unlock();
        m_condition_variable.notify_one();
    }

    std::optional<T> pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition_variable.wait(lock, [this]
        {
            return  m_Exiting || m_queue.size();
        });
        if(m_queue.empty() || m_Exiting)
        {
            return std::nullopt;
        }
        auto value = std::move(m_queue.front());
        m_queue.pop_front();
        lock.unlock();
        m_condition_variable.notify_one();
        return std::move(value);
    }

    void SetLimitQueue(const int LimitValue)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_LimitValue = LimitValue;
    }

    void release()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_Exiting = true;
        }
        m_condition_variable.notify_all();
    }

    void clear()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.clear();
        lock.unlock();
        m_condition_variable.notify_all();
    }

    int size()
    {
        std::shared_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }
private:
    std::size_t              m_LimitValue{};
    std::mutex              m_mutex;
    std::deque<T>           m_queue;
    bool                    m_Exiting = false;
    std::condition_variable m_condition_variable;
};

#endif