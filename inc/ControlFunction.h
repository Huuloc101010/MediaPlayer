#ifndef _CONTROL_FUNCTION_
#define _CONTROL_FUNCTION_

#include "Define.h"

class controlfunction
{
public:
    virtual void Play() ;
    virtual void Pause();
    virtual void Stop() ;
    virtual void Exit() ;
    virtual void CheckStateSleep();
    virtual bool CheckStateExit();

protected:
    std::atomic<PlayerState> m_PlayerState  = PlayerState::IDLE;
};

#endif /* _CONTROL_FUNCTION_ */