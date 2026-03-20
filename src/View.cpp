#include <cstring>
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

    if(m_Window.Init(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT) == false)
    {
        return false;
    }
    LOGI("Create windows success");
    if(m_VideoRenderer.Init(m_Window.Get(), DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT) == false)
    {
        return false;
    }
    LOGI("Create texture success ");
    return true;
}

void View::Config(const int Width, const int Height)
{
    std::lock_guard<std::mutex> lock(m_ResizeWindow);
    m_ConfigVideoSize = {Height, Width};
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
    if(m_CurrentVideoSize != m_ConfigVideoSize)
    {
        m_Window.Resize(m_ConfigVideoSize.Width, m_ConfigVideoSize.Height);
        m_VideoRenderer.Resize(m_Window.Get(), m_ConfigVideoSize.Width, m_ConfigVideoSize.Height);
        m_CurrentVideoSize = m_ConfigVideoSize;
    }
    if(m_CurrentVideoSize == m_ConfigVideoSize)
    {
        return;
    }
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
    // if Queue has no element -> Porcess other task
    if(!m_QueueSafe.Size())
    {
        return;
    }
    auto FrameOpt = m_QueueSafe.Pop();
    if(FrameOpt == std::nullopt)
    {
        LOGE("Null optional");
        return;
    }
    UniqueFramePtr frame = std::move(FrameOpt.value());
    //if(frame->format == AV_PIX_FMT_YUV420P)
    // double pts = frame->best_effort_timestamp * av_q2d(m_video_stream->time_base);
    // LOGI("video pts:{}", pts);
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

void View::PushVideoFrame(UniqueFramePtr frame)
{
    m_QueueSafe.Push(std::move(frame));
}