#include <SDL2/SDL.h>
#include "Controller.h"
#include "Log.h"
#include "Define.h"
#include "Mediator.h"

Controller::Controller(Mediator* Mediator) : m_Mediator(Mediator)
                                           , m_ThreadCheckEvent(&Controller::CheckEvent, this)

{
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
                    HandleKey(Event.key.keysym.sym);
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

void Controller::HandleKey(const SDL_Keycode KeyCode)
{
    PlayerEvent Event{};
    bool IsNeedPushEvent{};
    switch(KeyCode)
    {
        case SDLK_ESCAPE:
        {
            Event = PlayerEvent::QUIT;
            IsNeedPushEvent = true;
            break;
        }
        case SDLK_RIGHT:
        {
            Event = PlayerEvent::NEXT;
            IsNeedPushEvent = true;
            break;
        }
        case SDLK_LEFT:
        {
            Event = PlayerEvent::PRIVIOUS;
            IsNeedPushEvent = true;
            break;
        }
        case SDLK_SPACE:
        {
            if(m_PlayerState == PlayerState::PLAYING)
            {
                Event = PlayerEvent::PAUSE;
            }
            else
            {
                Event = PlayerEvent::PLAY;
            }
            IsNeedPushEvent = true;
            break;
        }
        case SDLK_UP:
        {
            Event = PlayerEvent::STOP;
            IsNeedPushEvent = true;
            break;
        }
        default:
        {
            IsNeedPushEvent = false;
            break;
        }
    }
    if(IsNeedPushEvent)
    {
        if(m_Mediator) m_Mediator->PushEvent(Event);
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

        case Rect::SEEK:
        {
            m_Mediator->PushEvent(PlayerEvent::SEEK);
            break;
        }

        default:
        {
            // Do not thing
            break;
        }
    }
}