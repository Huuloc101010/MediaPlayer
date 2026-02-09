#include <vector>
#include "videooutput.h"
#include "log.h"

videooutput::videooutput(const int width,const int height)
{
    this->width = width;
    this->height = height;
    init();  
}

videooutput::~videooutput()
{
    destroy();
}

bool videooutput::init()
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        LOGE("SDL_Init failed: {}", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow(NAME_WINDOW,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if(window == nullptr)
    {
        LOGE("Create window SDL fail: {}", SDL_GetError());
        return false;
    }
    LOGI("Create windows success");
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == nullptr)
    {
        std::cerr << "Create renderer SDL fail: " << SDL_GetError() << std::endl;
        LOGE("Create renderer SDL fail: {}", SDL_GetError());
        return false;
    }
    LOGI("create render success");
    texture = SDL_CreateTexture(renderer, 
        SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (texture == nullptr)
    {
        LOGE("Create texture SDL fail: {}", SDL_GetError());
        return false;
    }
    LOGI("Create texture success ");
    return true;
}

bool videooutput::show(const yuv& ndata)
{
        if(!checkevent())
        {
            exit(0);
        }
        // Push data to GPU
        if(SDL_UpdateYUVTexture(texture, NULL, 
            ndata.plane_y, ndata.linesize_y,           
            ndata.plane_u, ndata.linesize_u,       
            ndata.plane_v, ndata.linesize_v) < 0)
        {
            LOGE("update YUV texture fail");
            return false;
        }     

        if(SDL_RenderClear(renderer) < 0)
        {
            LOGE("render clear fail");
            return false;
        }
        if(SDL_RenderCopy(renderer, texture, NULL, NULL) < 0)
        {
            LOGE("Render copy fail");
            return false;
        }
        SDL_RenderPresent(renderer);
        return true;
}

void videooutput::destroy()
{
    if (texture)
    {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_Quit();
}

bool videooutput::checkevent()
{
    while(SDL_PollEvent(&e))
    {
        if(e.type == SDL_QUIT)
        {
            LOGW("Exitting");
            return false;
        }
    }
    return true;
}