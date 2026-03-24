#ifndef _CONTROLLER_
#define _CONTROLLER_

#include <thread>
#include <unordered_map>
#include <memory>
#include <Define.h>
#include "ControlFunction.h"

class Mediator;

class Controller : public ControlFunction
{
public:
    Controller(Mediator* Mediator);
    ~Controller() = default;
    void Exit() override;
private:
    void CheckEvent();
    void HandleClick(const Position Position);
    Mediator*          m_Mediator = nullptr;
    std::jthread       m_ThreadCheckEvent;

    std::unordered_map<SDL_Keycode, PlayerEvent> m_KeyCodeMap = {};
};

#endif /* _CONTROLLER */