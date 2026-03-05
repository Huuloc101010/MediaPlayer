#include "controlfunction.h"

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