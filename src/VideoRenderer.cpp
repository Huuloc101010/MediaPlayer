#include "VideoRenderer.h"

bool VideoRenderer::Init(const UniqueWindowPtr& Window, const int Width, const int Height)
{
    return CreateRenderer(Window) && CreateTexture(Width, Height);
}

bool VideoRenderer::CreateRenderer(const UniqueWindowPtr& Window)
{
    m_Renderer.reset(SDL_CreateRenderer(Window.get(), -1, SDL_RENDERER_ACCELERATED));
    if(m_Renderer == nullptr)
    {
        LOGE("Create renderer SDL fail: {}", SDL_GetError());
        return false;
    }
    return true;
}

bool VideoRenderer::CreateTexture(const int Width, const int Height) 
{
    m_Width = Width;
    m_Height = Height;
    m_Texture.reset(SDL_CreateTexture(m_Renderer.get(), 
        SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, Width, Height));
    if(m_Texture == nullptr)
    {
        LOGE("Create texture SDL fail: {}", SDL_GetError());
        return false;
    }
    return true;
}

bool VideoRenderer::Resize(const UniqueWindowPtr& Window,const int width, const int height)
{
    m_Texture.reset();
    m_Renderer.reset();
    return Init(Window, width, height);
}

bool VideoRenderer::UpdateYUVTexture(const yuv& ndata)
{
    if((m_Texture == nullptr) || (m_Renderer == nullptr))
    {
        LOGE("Fail to upadte YUV Texture");
        return false;
    }

    // Push data to GPU
    if(SDL_UpdateYUVTexture(m_Texture.get(), NULL, 
        ndata.plane_y, ndata.linesize_y,           
        ndata.plane_u, ndata.linesize_u,       
        ndata.plane_v, ndata.linesize_v) < 0)
    {
        LOGE("update YUV texture fail");
        return false;
    }     

    if(SDL_RenderClear(m_Renderer.get()) < 0)
    {
        LOGE("render clear fail");
        return false;
    }
    if(SDL_RenderCopy(m_Renderer.get(), m_Texture.get(), NULL, NULL) < 0)
    {
        LOGE("Render copy fail");
        return false;
    }
    
    SDL_RenderPresent(m_Renderer.get());
    return true;
}