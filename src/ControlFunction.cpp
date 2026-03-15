#include "ControlFunction.h"
#include "thread"

void ControlFunction::Play()
{
    m_PlayerState = PlayerState::PLAYING;
    LOGE("Status Playing");
    m_PlayerState.notify_all();
}

void ControlFunction::Pause()
{
    m_PlayerState = PlayerState::PAUSED;
    LOGE("Status Pause");
    m_PlayerState.notify_all();
}

void ControlFunction::Stop()
{
    m_PlayerState = PlayerState::STOPPED;
    LOGE("Status Stop");
    m_PlayerState.notify_all();
}

void ControlFunction::Exit()
{
    m_PlayerState = PlayerState::EXITING;
    LOGE("Status Exitinng");
    m_PlayerState.notify_all();
}

void ControlFunction::CheckStateSleep()
{
    while(true)
    {
        PlayerState CurrentState = m_PlayerState.load();
        if((CurrentState != PlayerState::IDLE)
        && (CurrentState != PlayerState::PAUSED))
        {
            break;
        }
        m_PlayerState.wait(CurrentState);
    }
}

bool ControlFunction::CheckStateExit()
{
    PlayerState CurrentState = m_PlayerState.load();
    return (CurrentState == PlayerState::EXITING) || (CurrentState == PlayerState::STOPPED);
}
