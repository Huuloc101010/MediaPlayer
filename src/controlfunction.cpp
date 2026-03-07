#include "controlfunction.h"
#include "thread"

void controlfunction::Play()
{
    m_PlayerState = PlayerState::PLAYING;
    m_PlayerState.notify_all();
}

void controlfunction::Pause()
{
    m_PlayerState = PlayerState::PAUSED;
    m_PlayerState.notify_all();
}

void controlfunction::Stop()
{
    m_PlayerState = PlayerState::STOPPED;
    m_PlayerState.notify_all();
}

void controlfunction::Exit()
{
    m_PlayerState = PlayerState::EXITING;
    m_PlayerState.notify_all();
}

void controlfunction::CheckStateSleep()
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

bool controlfunction::CheckStateExit()
{
    PlayerState CurrentState = m_PlayerState.load();
    return (CurrentState == PlayerState::EXITING) || (CurrentState == PlayerState::STOPPED);
}
