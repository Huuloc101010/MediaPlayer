#include <cstring>
#include "view.h"

view::view()
{
    
}

view::~view()
{
    SDLStop();
    //SDL_Quit();
}


bool view::init()
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        LOGE("SDL_Init failed: {}", SDL_GetError());
        return false;
    }

    m_Window.reset(SDL_CreateWindow(NAME_WINDOW,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_Width, m_Height, SDL_WINDOW_SHOWN));
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

void view::Config(const int Width, const int Height)
{
    m_Width = Width;
    m_Height = Height;
}

bool view::UpdateYUVTexture(const yuv& ndata)
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

bool view::config(int sample_rate,
                         int channels,
                         SDL_AudioFormat format,
                         int first_pts,
                         int samples)
{
    m_SampleRate = sample_rate;
    m_FirstPts = first_pts;
    m_Sample = samples;
    SDL_AudioSpec want{};
    want.freq = sample_rate;
    want.channels = channels;
    want.format = format;
    want.samples = samples;
    want.callback = [](void* userdata, Uint8* stream, int len)
    {static_cast<view*>(userdata)->callback(stream, len);};
    want.userdata = this;
    m_DeviceId = SDL_OpenAudioDevice(nullptr, 0, &want, &m_Spec, 0);
    LOGI("Open audio device success: {}", m_DeviceId);
    if(!m_DeviceId)
    {
        LOGE("SDL_OpenAudioDevice failed: {}", SDL_GetError());
        return false;
    }
    view::Play();
    return true;
}


void view::Play()
{
    controlfunction::Play();
    view::SDLStart();
    LOGE("view::play");
}

void view::Pause()
{
    controlfunction::Pause();
    view::SDLPause();
}

void view::Stop()
{
    controlfunction::Stop();
    clear();
    view::SDLStop();
    SDL_Quit();
}

void view::Exit()
{
    controlfunction::Exit();
    view::SDLStop();
    clear();
}

void view::SDLStart()
{
    LOGE("SDL start");
    if(m_DeviceId)
    {
        SDL_PauseAudioDevice(m_DeviceId, 0);
    }
}

void view::SDLPause()
{
    if(m_DeviceId)
    {
        SDL_PauseAudioDevice(m_DeviceId, 1);
    }
}

void view::SDLStop()
{
    if(m_DeviceId)
    {
        SDL_PauseAudioDevice(m_DeviceId, 1);
        SDL_CloseAudioDevice(m_DeviceId);
        m_DeviceId = 0;
    }
}

void view::clear()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Deque.clear();
}

void view::push(const uint8_t* data, size_t size)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Deque.insert(m_Deque.end(), data, data + size);
}

void view::sdl_callback(void* userdata, Uint8* stream, int len)
{
    if((userdata == nullptr) || (stream == nullptr))
    {
        return;
    }
    static_cast<view*>(userdata)->callback(stream, len);
}

void view::callback(Uint8* stream, int len)
{
   // LOGE("callback called");
    std::memset(stream, 0, len); // silence if not enable data

    std::lock_guard<std::mutex> lock(m_Mutex);

    int to_copy = std::min<int>(len, m_Deque.size());
    for(int i = 0; i < to_copy; ++i) 
    {
        stream[i] = m_Deque.front();
        m_Deque.pop_front();
    }

    /* calculate audio timestamp */

    m_TotalSamplePlayed += m_Sample;
    m_Clock.last_frame_pts.store(m_Clock.pts);
    m_Clock.pts = m_FirstPts + (static_cast<double>(m_TotalSamplePlayed) / m_SampleRate);
    //LOGW("audio clock = {}", m_Clock.pts.load());
}



double view::get_clock()
{
    return m_Clock.pts;
}