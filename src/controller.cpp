#include <SDL2/SDL.h>
#include "controller.h"
#include "log.h"
#include "define.h"
#include "mediator.h"

controller::controller(mediator* mediator) : m_Mediator(mediator)
                                           , m_ThreadCheckEvent(&controller::checkevent, this)

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

void controller::checkevent()
{
    SDL_Event Event{};
    while(m_PlayerState.load() != PlayerState::EXITING)
    {
        while((SDL_PollEvent(&Event)) && m_PlayerState.load() != PlayerState::EXITING)
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

void controller::Exit()
{
    controlfunction::Exit();
    if(m_ThreadCheckEvent.joinable())
    {
        m_ThreadCheckEvent.join();
    }
}