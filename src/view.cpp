#include "view.h"

view::view()
{
    
}

view::~view()
{
    destroy();
}

bool view::init()
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        LOGE("SDL_Init failed: {}", SDL_GetError());
        return false;
    }

    m_Window = SDL_CreateWindow(NAME_WINDOW,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_Width, m_Height, SDL_WINDOW_SHOWN);
    if(m_Window == nullptr)
    {
        LOGE("Create window SDL fail: {}", SDL_GetError());
        return false;
    }
    LOGI("Create windows success");
    m_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_ACCELERATED);
    if(m_Renderer == nullptr)
    {
        std::cerr << "Create renderer SDL fail: " << SDL_GetError() << std::endl;
        LOGE("Create renderer SDL fail: {}", SDL_GetError());
        return false;
    }
    LOGI("create render success");
    m_Texture = SDL_CreateTexture(m_Renderer, 
        SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, m_Width, m_Height);
    if (m_Texture == nullptr)
    {
        LOGE("Create texture SDL fail: {}", SDL_GetError());
        return false;
    }
    LOGI("Create texture success ");
    return true;
}

void view::Config(const int Width, const int Height)
{
    m_Width = Width;
    m_Height = Height;
}

bool view::UpdateYUVTexture(const yuv& ndata)
{
    // Push data to GPU
    if(SDL_UpdateYUVTexture(m_Texture, NULL, 
        ndata.plane_y, ndata.linesize_y,           
        ndata.plane_u, ndata.linesize_u,       
        ndata.plane_v, ndata.linesize_v) < 0)
    {
        LOGE("update YUV texture fail");
        return false;
    }     

    if(SDL_RenderClear(m_Renderer) < 0)
    {
        LOGE("render clear fail");
        return false;
    }
    if(SDL_RenderCopy(m_Renderer, m_Texture, NULL, NULL) < 0)
    {
        LOGE("Render copy fail");
        return false;
    }
    SDL_RenderPresent(m_Renderer);
    return true;
}


void view::destroy()
{
    if (m_Texture)
    {
        SDL_DestroyTexture(m_Texture);
        m_Texture = nullptr;
    }
    if (m_Renderer)
    {
        SDL_DestroyRenderer(m_Renderer);
        m_Renderer = nullptr;
    }
    if (m_Window)
    {
        SDL_DestroyWindow(m_Window);
        m_Window = nullptr;
    }
    SDL_Quit();
}
