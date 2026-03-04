#include <SDL2/SDL.h>
#include "controller.h"
#include "log.h"
#include "define.h"
#include "mediator.h"

controller::controller(mediator* mediator) : m_Mediator(mediator)
                                           , m_ThreadCheckEvent(&controller::checkevent, this)

{

}

void controller::checkevent()
{
    SDL_Event Event;
    while(true)
    {
        while(SDL_PollEvent(&Event))
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
                    switch(Event.key.keysym.sym)
                    {
                        case SDLK_SPACE:
                        {
                            if(m_Mediator) m_Mediator->PushEvent(PlayerEvent::STOP);
                            break;
                        }

                        case SDLK_RIGHT:
                        {
                            if(m_Mediator) m_Mediator->PushEvent(PlayerEvent::NEXT);
                            break;
                        }
                    }
                    break;
                }
            }
        }
        // decrease cpu workload
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}