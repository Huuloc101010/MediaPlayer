#include <cstring>
#include "View.h"

View::View()
{
    
}

View::~View()
{
    m_AudioDevice.SDLStop();
    //SDL_Quit();
}


bool View::Init()
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        LOGE("SDL_Init failed: {}", SDL_GetError());
        return false;
    }

    m_Window.reset(SDL_CreateWindow(NAME_WINDOW,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_Width, m_Height, SDL_WINDOW_RESIZABLE));
    if(m_Window == nullptr)
    {
        LOGE("Create window SDL fail: {}", SDL_GetError());
        return false;
    }
    LOGI("Create windows success");
    m_Renderer.reset(SDL_CreateRenderer(m_Window.get(), -1, SDL_RENDERER_ACCELERATED));
    if(m_Renderer == nullptr)
    {
        std::cerr << "Create renderer SDL fail: " << SDL_GetError() << std::endl;
        LOGE("Create renderer SDL fail: {}", SDL_GetError());
        return false;
    }
    LOGI("create render success");
    m_Texture.reset(SDL_CreateTexture(m_Renderer.get(), 
        SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, m_Width, m_Height));
    if (m_Texture == nullptr)
    {
        LOGE("Create texture SDL fail: {}", SDL_GetError());
        return false;
    }
    LOGI("Create texture success ");
    return true;
}

void View::Config(const int Width, const int Height)
{
    m_Width = Width;
    m_Height = Height;
}

bool View::UpdateYUVTexture(const yuv& ndata)
{
    if(m_PlayerState.load() != PlayerState::PLAYING)
    {
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

bool View::Config(int sample_rate,
                         int channels,
                         SDL_AudioFormat format,
                         int first_pts,
                         int samples)
{
    m_AudioDevice.Config(sample_rate, channels, format, first_pts, samples);
    View::Play();
    return true;
}


void View::Play()
{
    ControlFunction::Play();
    m_AudioDevice.SDLStart();
    LOGE("View::play");
}

void View::Pause()
{
    ControlFunction::Pause();
    m_AudioDevice.SDLPause();
}

void View::Stop()
{
    ControlFunction::Stop();
    m_AudioDevice.Clear();
    m_AudioDevice.SDLStop();
    SDL_Quit();
}

void View::Exit()
{
    ControlFunction::Exit();
    m_AudioDevice.SDLStop();
    m_AudioDevice.Clear();
}

void View::Push(const uint8_t* data, size_t Size)
{
    m_AudioDevice.Push(data, Size);
}

std::atomic<double>& View::GetClock()
{
    return m_AudioDevice.GetClock();
}