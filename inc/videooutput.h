#ifndef _VIDEO_OUTPUT_
#define _VIDEO_OUTPUT_

#include <mutex>
#include <thread>
#include <shared_mutex>
#include <SDL2/SDL.h>
#include <iostream>
#include <memory>
#include "define.h"

class mediator;

class videooutput
{
public:
    videooutput(const int width,const int height, mediator* mediator);
    ~videooutput();
    bool show(const yuv& ndata);
    bool show2(AVFrame* frame);
    void push_queue(UniqueFramePtr framePtr);
    UniqueFramePtr pop_queue();

private:
    void checkevent();
    bool init();
    void destroy();
    mediator*            m_mediator{};
    queue_safe           m_QueueSafe{};
    std::atomic<bool>    m_exiting = false;
    std::thread          m_ThreadCheckEvent;
    int                  m_width = 0;
    int                  m_height = 0;
    SDL_Window*          m_window = nullptr;
    SDL_Renderer*        m_renderer = nullptr;
    SDL_Texture*         m_texture = nullptr;
    SDL_Event            m_event;
};

#endif /*_VIDEO_OUTPUT_*/