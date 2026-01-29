#include <vector>
#include "videooutput.h"

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

int videooutput::init()
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    window = SDL_CreateWindow("YUV Dynamic Video",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if(window == nullptr)
    {
        std::cerr << "Create window SDL fail: " << SDL_GetError() << std::endl;
        return -1;
    }
    std::cout << "Create windows success" << std::endl;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == nullptr)
    {
        std::cerr << "Create renderer SDL fail: " << SDL_GetError() << std::endl;
        return -1;
    }
    std::cout << "create render success" << std::endl;
    texture = SDL_CreateTexture(renderer, 
        SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (texture == nullptr)
    {
        std::cerr << "Create texture SDL fail: " << SDL_GetError() << std::endl;
        return -1;
    }
    std::cout << "Create texture success " << std::endl;
    return 0;
}

int videooutput::show(const yuv& ndata)
{
        if(!checkevent())
        {
            return false;
        }
        // Push data to GPU
        SDL_UpdateYUVTexture(texture, NULL, 
            ndata.plane_y, width,           
            ndata.plane_u, width / 2,       
            ndata.plane_v, width / 2);      

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
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

int videooutput::checkevent()
{
    while(SDL_PollEvent(&e))
    {
        if(e.type == SDL_QUIT)
        {
            std::cout << "Exitting" << std::endl;
            return false;
        }
    }
    return true;
}