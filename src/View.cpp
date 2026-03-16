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

    if(m_Window.Init(m_Width, m_Height) == false)
    {
        return false;
    }
    LOGI("Create windows success");
    if(m_VideoRenderer.Init(m_Window.Get(), m_Width, m_Height) == false)
    {
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
    return m_VideoRenderer.UpdateYUVTexture(ndata);
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