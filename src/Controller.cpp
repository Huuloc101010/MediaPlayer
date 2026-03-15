#include <SDL2/SDL.h>
#include "Controller.h"
#include "Log.h"
#include "Define.h"
#include "Mediator.h"

Controller::Controller(Mediator* Mediator) : m_Mediator(Mediator)
                                           , m_ThreadCheckEvent(&Controller::checkevent, this)

{
    m_KeyCodeMap =
    {
        {SDLK_ESCAPE,PlayerEvent::QUIT},
        {SDLK_UP,    PlayerEvent::STOP},
        {SDLK_RIGHT, PlayerEvent::NEXT},
        {SDLK_LEFT,  PlayerEvent::PAUSE},
        {SDLK_DOWN,  PlayerEvent::PLAY}
    };
}

void Controller::checkevent()
{
    SDL_Event Event{};
    while((m_PlayerState.load() != PlayerState::EXITING) && (m_PlayerState.load() != PlayerState::STOPPED))
    {
        while((m_PlayerState.load() != PlayerState::EXITING) && (m_PlayerState.load() != PlayerState::STOPPED) && (SDL_PollEvent(&Event)))
        {
            switch(Event.type)
            {
                case SDL_QUIT:
                {
                    if(m_Mediator) m_Mediator->PushEvent(PlayerEvent::QUIT);
                    break;
                }

                case SDL_KEYDOWN:
                {
                    if(m_KeyCodeMap.count(Event.key.keysym.sym))
                    {
                        PlayerEvent PlayerEvent = m_KeyCodeMap[Event.key.keysym.sym];
                        if(m_Mediator) m_Mediator->PushEvent(PlayerEvent);
                    }
                    break;
                }
            }
        }
        // decrease cpu workload
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Controller::Exit()
{
    ControlFunction::Exit();
    if(m_ThreadCheckEvent.joinable())
    {
        m_ThreadCheckEvent.join();
    }
}