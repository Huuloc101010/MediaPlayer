#ifndef _CONTROL_FUNCTION_
#define _CONTROL_FUNCTION_

#include "Define.h"

class ControlFunction
{
public:
    virtual void Play() ;
    virtual void Pause();
    virtual void Stop() ;
    virtual void Exit() ;
    virtual void CheckStateSleep();
    virtual bool CheckStateExit();
    virtual void FlushData() {};

protected:
    std::atomic<PlayerState> m_PlayerState  = PlayerState::IDLE;
};

#endif /* _CONTROL_FUNCTION_ */