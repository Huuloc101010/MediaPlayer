#ifndef _CONTROL_FUNCTION_
#define _CONTROL_FUNCTION_

#include "define.h"

class controlfunction
{
    virtual void Play() ;
    virtual void Pause();
    virtual void Stop() ;
    virtual void Exit() ;

protected:
    std::atomic<PlayerState> m_PlayerState  = PlayerState::IDLE;
};

#endif /* _CONTROL_FUNCTION_ */