#ifndef _VIDEO_OUTPUT_
#define _VIDEO_OUTPUT_

#include <mutex>
#include <thread>
#include <shared_mutex>
#include <SDL2/SDL.h>
#include <iostream>
#include "define.h"

class videooutput
{
public:
    videooutput(const int width,const int height);
    ~videooutput();
    bool init();
    void destroy();
    bool show(const yuv& ndata);
    bool checkevent();
private:
    int width;
    int height;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;
    SDL_Event e;
};

#endif /*_VIDEO_OUTPUT_*/