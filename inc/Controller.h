#ifndef _CONTROLLER_
#define _CONTROLLER_

#include <thread>
#include <unordered_map>
#include <memory>
#include <Define.h>
#include "ControlFunction.h"

class mediator;

class controller : public controlfunction
{
public:
    controller(mediator* mediator);
    ~controller() = default;
    void Exit() override;
private:
    void checkevent();
    mediator*          m_Mediator = nullptr;
    std::jthread       m_ThreadCheckEvent;

    std::unordered_map<SDL_Keycode, PlayerEvent> m_KeyCodeMap = {};
};

#endif /* _CONTROLLER */