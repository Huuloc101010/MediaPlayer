#ifndef _VIDEO_OUTPUT_
#define _VIDEO_OUTPUT_

#include <mutex>
#include <shared_mutex>
#include <SDL2/SDL.h>
#include <iostream>
#include "define.h"

class videooutput
{
public:
    videooutput(const int width,const int height);
    ~videooutput();
    int init();
    void destroy();
    int show(const yuv& ndata);
    int checkevent();
private:
    int width;
    int height;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Event e;
};

#endif /*_VIDEO_OUTPUT_*/