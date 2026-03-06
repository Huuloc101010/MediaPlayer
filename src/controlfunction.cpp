#include "controlfunction.h"
#include "thread"

void controlfunction::Play()
{
    m_PlayerState = PlayerState::PLAYING;
}

void controlfunction::Pause()
{
    m_PlayerState = PlayerState::PAUSED;
}

void controlfunction::Stop()
{
    m_PlayerState = PlayerState::STOPPED;
}

void controlfunction::Exit()
{
    m_PlayerState = PlayerState::EXITING;
}

void controlfunction::CheckStateSleep()
{
    static const std::unordered_set<PlayerState> SetStateWait =
    {
        PlayerState::IDLE,
        PlayerState::PAUSED,
    };
    while(SetStateWait.count(m_PlayerState.load()))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

bool controlfunction::CheckStateExit()
{
    static const std::unordered_set<PlayerState> SetStateExit =
    {
        PlayerState::EXITING,
        PlayerState::STOPPED,
    };
    return SetStateExit.count(m_PlayerState.load());
}
