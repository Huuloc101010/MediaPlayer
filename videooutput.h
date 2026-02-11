#ifndef _VIDEO_OUTPUT_
#define _VIDEO_OUTPUT_

#include <mutex>
#include <thread>
#include <shared_mutex>
#include <SDL2/SDL.h>
#include <iostream>
#include <memory>
#include "define.h"

class videooutput
{
public:
    videooutput(const int width,const int height);
    ~videooutput();
    bool show(const yuv& ndata);
private:
    void checkevent();
    bool init();
    void destroy();
    std::atomic<bool> m_exiting = false;
    std::thread m_ThreadCheckEvent;
    int width;
    int height;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    SDL_Event m_event;
};

#endif /*_VIDEO_OUTPUT_*/