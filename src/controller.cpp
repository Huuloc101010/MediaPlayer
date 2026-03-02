#include <SDL2/SDL.h>
#include "controller.h"
#include "log.h"

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
            if(Event.type == SDL_QUIT)
            {
                LOGI("Event SDL_QUIT");
                LOGW("Exitting");
                exit(0);
            }
        }
        // decrease cpu workload
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}