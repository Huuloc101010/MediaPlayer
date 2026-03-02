#ifndef _CONTROLLER_
#define _CONTROLLER_

#include <thread>

class mediator;

class controller
{
public:
    controller(mediator* mediator);
    ~controller() = default;
private:
    void checkevent();
    mediator*          m_Mediator = nullptr;
    std::jthread       m_ThreadCheckEvent;
};

#endif /* _CONTROLLER */