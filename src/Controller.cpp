#include <SDL2/SDL.h>
#include "Controller.h"
#include "Log.h"
#include "Define.h"
#include "Mediator.h"

Controller::Controller(Mediator* Mediator) : m_Mediator(Mediator)
                                           , m_ThreadCheckEvent(&Controller::CheckEvent, this)

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

void Controller::CheckEvent()
{
    SDL_Event Event{};
    while((m_PlayerState.load() != PlayerState::EXITING))
    {
        while((m_PlayerState.load() != PlayerState::EXITING) && (SDL_PollEvent(&Event)))
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

                case SDL_MOUSEBUTTONDOWN:
                {
                    LOGI("Received event mouse click");
                    HandleClick({Event.button.x, Event.button.y});
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

void Controller::HandleClick(const Position Position)
{
    LOGD("Received event x:{}, y:{}", Position.x, Position.y);
    if(m_Mediator == nullptr)
    {
        return;
    }
    Rect CurrentRect = m_Mediator->CheckInWhichButton(Position);
    switch(CurrentRect)
    {
        case Rect::NEXT:
        {
            m_Mediator->PushEvent(PlayerEvent::NEXT);
            break;
        }

        case Rect::PRIVIOUS:
        {
            m_Mediator->PushEvent(PlayerEvent::PRIVIOUS);
            break;
        }

        case Rect::PLAY:
        {
            if(m_PlayerState == PlayerState::PLAYING)
            {
                m_Mediator->PushEvent(PlayerEvent::PAUSE);
            }
            else
            {
                m_Mediator->PushEvent(PlayerEvent::PLAY);
            }
            break;
        }

        default:
        {
            // Do not thing
            break;
        }
    }
}