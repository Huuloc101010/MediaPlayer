#include "View.h"

View::View()
{
    Init();
    m_CurrentVideoSize = {DEFAULT_WINDOW_HEIGHT, DEFAULT_WINDOW_WIDTH};
    m_ConfigVideoSize  = m_CurrentVideoSize;
}

View::~View()
{
    m_AudioDevice.SDLStop();
    m_QueueSafe.Release();
    SDL_Quit();
}


bool View::Init()
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        LOGE("SDL_Init failed: {}", SDL_GetError());
        return false;
    }

    if(m_Window.Init({DEFAULT_WINDOW_HEIGHT, DEFAULT_WINDOW_WIDTH}) == false)
    {
        return false;
    }
    LOGI("Create windows success");
    if(m_VideoRenderer.Init(m_Window.Get(), {DEFAULT_WINDOW_HEIGHT, DEFAULT_WINDOW_WIDTH}) == false)
    {
        return false;
    }
    m_MaxWindowSize = GetMaxWindowSize();
    LOGI("Create texture success ");
    return true;
}

void View::Config(const Size WindowSize)
{
    std::lock_guard<std::mutex> lock(m_ResizeWindow);
    m_ConfigVideoSize = WindowSize;
    m_CurrentWindowSize = m_ConfigVideoSize;
}


bool View::Config(int sample_rate,
                         int channels,
                         SDL_AudioFormat format,
                         int first_pts,
                         int samples)
{
    m_AudioDevice.Config(sample_rate, channels, format, first_pts, samples);
    m_AudioDevice.SDLStart();
    return true;
}

void View::CheckResizeWindow()
{
    std::lock_guard<std::mutex> lock(m_ResizeWindow);
    if(m_CurrentVideoSize == m_ConfigVideoSize)
    {
        return;
    }
    m_VideoRenderer.Resize(m_Window.Get(), m_ConfigVideoSize);
    m_CurrentVideoSize = m_ConfigVideoSize;
    m_CurrentWindowSize = m_ConfigVideoSize;
}

void View::Play()
{
    ControlFunction::Play();
    m_AudioDevice.SDLStart();
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
}

void View::Exit()
{
    m_AudioDevice.SDLPause();
    m_AudioDevice.Clear();
    m_AudioDevice.ClearAudioPts();
    m_QueueSafe.Clear();
}

void View::Push(const uint8_t* data, size_t Size)
{
    m_AudioDevice.Push(data, Size);
}

std::atomic<double>& View::GetClock()
{
    return m_AudioDevice.GetClock();
}

void View::ShowVideo()
{
    if(m_VideoRenderer.RenderClear() == false)
    {
        LOGE("render clear fail");
    }
    if(m_QueueSafe.Size())
    {
        auto FrameOpt = m_QueueSafe.Pop();
        if(FrameOpt == std::nullopt)
        {
            LOGE("Null optional");
            return;
        }
        UniqueFramePtr frame = std::move(FrameOpt.value());
        yuv lyuv =
        {
            frame->data[0],
            frame->data[1],
            frame->data[2],
            frame->linesize[0],
            frame->linesize[1],
            frame->linesize[2]
        };
        if(m_VideoRenderer.UpdateYUVTexture(lyuv) == false)
        {
            LOGE("Update YUV Texture fail");
        }
    }
    
    CalculateRect(m_Window.GetCurrentWindowSize());

    if(m_VideoRenderer.RenderCopyVideoTexture(m_VideoRect) == false)
    {
        LOGE("render video texture fail");
    }

    if(m_VideoRenderer.RenderCopyButtonPlay(m_ButtonPlayRect) == false)
    {
        LOGE("render button play fail");
    }

    if(m_VideoRenderer.RenderCopyButtonNext(m_ButtonNextRect) == false)
    {
        LOGE("render button next fail");
    }
    if(m_VideoRenderer.RenderCopyButtonPrivious(m_ButtonPriviousRect) == false)
    {
        LOGE("render button privious fail");
    }

    m_VideoRenderer.Present();
}

void View::PushVideoFrame(UniqueFramePtr frame)
{
    m_QueueSafe.Push(std::move(frame));
}

Size View::GetMaxWindowSize()
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

void View::CalculateRect(const Size CurrentWindowSize)
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

bool View::CheckInRect(const SDL_Rect& rect, const Position position)
{
    return (position.x >= rect.x)
        && (position.x <= (rect.x + rect.w))
        && (position.y >= rect.y)
        && (position.y <= (rect.y + rect.h));
}

Rect View::CheckInWhichButton(const Position position)
{
    if(CheckInRect(m_ButtonPlayRect, position))    return Rect::PLAY;
    if(CheckInRect(m_ButtonNextRect, position))    return Rect::NEXT;
    if(CheckInRect(m_ButtonPriviousRect, position))return Rect::PRIVIOUS;
    return Rect::NONE;
}