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

bool videooutput::init()
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow(NAME_WINDOW,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if(window == nullptr)
    {
        std::cerr << "Create window SDL fail: " << SDL_GetError() << std::endl;
        return false;
    }
    std::cout << "Create windows success" << std::endl;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == nullptr)
    {
        std::cerr << "Create renderer SDL fail: " << SDL_GetError() << std::endl;
        return false;
    }
    std::cout << "create render success" << std::endl;
    texture = SDL_CreateTexture(renderer, 
        SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (texture == nullptr)
    {
        std::cerr << "Create texture SDL fail: " << SDL_GetError() << std::endl;
        return false;
    }
    std::cout << "Create texture success " << std::endl;
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
            ndata.plane_y, width,           
            ndata.plane_u, width / 2,       
            ndata.plane_v, width / 2) < 0)
        {
            std::cerr << "update YUV texture fail" << std::endl;
            return false;
        }     

        if(SDL_RenderClear(renderer) < 0)
        {
            std::cerr << "render clear fail" << std::endl;
            return false;
        }
        if(SDL_RenderCopy(renderer, texture, NULL, NULL) < 0)
        {
            std::cerr << "Render copy fail" << std::endl;
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
            std::cout << "Exitting" << std::endl;
            return false;
        }
    }
    return true;
}