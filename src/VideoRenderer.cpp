#include "VideoRenderer.h"

bool VideoRenderer::Init(const UniqueWindowPtr& Window, const Size VideoSize)
{
    return CreateRenderer(Window) && CreateVideoTexture(VideoSize);
}

bool VideoRenderer::CreateRenderer(const UniqueWindowPtr& Window)
{
    // Create video renderer
    m_Renderer.reset(SDL_CreateRenderer(Window.get(), -1, SDL_RENDERER_ACCELERATED));
    if(m_Renderer == nullptr)
    {
        LOGE("Create renderer SDL fail: {}", SDL_GetError());
        return false;
    }

    // Create button play texture
    m_ButtonPlay.reset(IMG_LoadTexture(m_Renderer.get(), ICON_BUTTON_PLAY_PATH));
    if(m_ButtonPlay == nullptr)
    {
        LOGE("Load image fail");
        return false;
    }

    // Create button next texture
    m_ButtonNext.reset(IMG_LoadTexture(m_Renderer.get(), ICON_BUTTON_NEXT_PATH));
    if(m_ButtonNext == nullptr)
    {
        LOGE("Load image fail");
        return false;
    }

    // Create button privious texture
    m_ButtonPrivious.reset(IMG_LoadTexture(m_Renderer.get(), ICON_BUTTON_PRIVIOUS_PATH));
    if(m_ButtonNext == nullptr)
    {
        LOGE("Load image fail");
        return false;
    }

    return true;
}

bool VideoRenderer::CreateVideoTexture(const Size VideoSize) 
{
    m_CurrentVideoSize = VideoSize;
    m_VideoTexture.reset(SDL_CreateTexture(m_Renderer.get(), 
        SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, VideoSize.Width, VideoSize.Height));
    if(m_VideoTexture == nullptr)
    {
        LOGE("Create texture SDL fail: {}", SDL_GetError());
        return false;
    }
    return true;
}

bool VideoRenderer::Resize(const UniqueWindowPtr& Window,const Size VideoSize)
{
    (void)VideoSize;
    m_CurrentVideoSize = VideoSize;
    m_VideoTexture.reset();
    m_Renderer.reset();
    Size CurrentWindowSize {};
    SDL_GetWindowSize(Window.get(), &CurrentWindowSize.Width, &CurrentWindowSize.Height);
    CalculateRect(CurrentWindowSize);
    return Init(Window, m_CurrentVideoSize);
}

Size VideoRenderer::GetMaxWindowSize()
{
    Size Retval{};
    SDL_DisplayMode mode;
    if (SDL_GetCurrentDisplayMode(0, &mode) != 0)
    {
        LOGE("SDL_GetCurrentDisplayMode failed: {}", SDL_GetError());
    }
    else
    {
        Retval.Width  = mode.w;
        Retval.Height = mode.h;
        LOGI("Max screen size: {}x{}", Retval.Width, Retval.Height);
    }
    return Retval;
}

void VideoRenderer::CalculateRect(const Size CurrentWindowSize)
{
    // Calculating scale retio
    double ratio = std::min(((double)CurrentWindowSize.Width) / m_CurrentVideoSize.Width,
                            ((double)CurrentWindowSize.Height - DEFAULT_WINDOW_CONTROL) / m_CurrentVideoSize.Height);
    if(ratio > 1.0)
    {
        ratio = 1.0;
    }

    // Creat Renderer same size with Window
    m_VideoRect.x = (CurrentWindowSize.Width - m_CurrentVideoSize.Width * ratio) / 2;
    m_VideoRect.h = (m_CurrentVideoSize.Height * ratio);
    m_VideoRect.w = m_CurrentVideoSize.Width * ratio;
    m_ControlAreaRect = {0, CurrentWindowSize.Height - DEFAULT_WINDOW_CONTROL, CurrentWindowSize.Width, DEFAULT_WINDOW_CONTROL};
    // button play
    m_ButtonPlayRect.x = (CurrentWindowSize.Width - 50) / 2;
    m_ButtonPlayRect.y = m_ControlAreaRect.y + (DEFAULT_WINDOW_CONTROL - 100) / 2;
    m_ButtonPlayRect.w = DEFAULT_BUTTON_WIDTH;
    m_ButtonPlayRect.h = DEFAULT_BUTTON_HEIGHT;

    // button next
    m_ButtonNextRect.x = m_ButtonPlayRect.x + 100;
    m_ButtonNextRect.y = m_ControlAreaRect.y + (DEFAULT_WINDOW_CONTROL - 100) / 2;
    m_ButtonNextRect.w = DEFAULT_BUTTON_WIDTH;
    m_ButtonNextRect.h = DEFAULT_BUTTON_HEIGHT;

    // button prious
    m_ButtonPriviousRect.x = m_ButtonPlayRect.x - 100;
    m_ButtonPriviousRect.y = m_ControlAreaRect.y + (DEFAULT_WINDOW_CONTROL - 100) / 2;
    m_ButtonPriviousRect.w = DEFAULT_BUTTON_WIDTH;
    m_ButtonPriviousRect.h = DEFAULT_BUTTON_HEIGHT;
}

bool VideoRenderer::UpdateYUVTexture(const yuv& ndata)
{
    if((m_VideoTexture == nullptr) || (m_Renderer == nullptr))
    {
        LOGE("Fail to upadte YUV Texture");
        return false;
    }
    // Push data to GPU
    if(SDL_UpdateYUVTexture(m_VideoTexture.get(), nullptr, 
        ndata.plane_y, ndata.linesize_y,           
        ndata.plane_u, ndata.linesize_u,       
        ndata.plane_v, ndata.linesize_v) < 0)
    {
        LOGE("update YUV texture fail");
        return false;
    }     
    return true;
}

bool VideoRenderer::RenderClear()
{
    if(SDL_RenderClear(m_Renderer.get()) < 0)
    {
        LOGE("render clear fail");
        return false;
    }
    return true;
}

bool VideoRenderer::RenderCopyVideoTexture(const SDL_Rect& Rect)
{
    return (SDL_RenderCopy(m_Renderer.get(), m_VideoTexture.get(), NULL, &Rect) == 0);
}

bool VideoRenderer::RenderCopyButtonPlay(const SDL_Rect& Rect)
{
    return (SDL_RenderCopy(m_Renderer.get(), m_ButtonPlay.get(), NULL, &Rect) == 0);
}

bool VideoRenderer::RenderCopyButtonNext(const SDL_Rect& Rect)
{
    return (SDL_RenderCopy(m_Renderer.get(), m_ButtonNext.get(), NULL, &Rect) == 0);
}

bool VideoRenderer::RenderCopyButtonPrivious(const SDL_Rect& Rect)
{
    return (SDL_RenderCopy(m_Renderer.get(), m_ButtonPrivious.get(), NULL, &Rect) == 0);
}

void VideoRenderer::Present()
{
    SDL_RenderPresent(m_Renderer.get());
}