#ifndef _VIEW_
#define _VIEW_

#include <SDL2/SDL.h>
#include "define.h"
#include "controlfunction.h"

class view : public controlfunction
{
public:
    view();
    ~view();
    bool init();
    void Config(const int Width, const int Height);
    bool UpdateYUVTexture(const yuv& ndata);
    void Play() override;
    void Pause()override;
    void Stop() override;
    void Exit() override;

    void SDLStart();
    void SDLPause();
    void SDLStop();

    void push(const uint8_t* data, size_t size);
    void clear();
    
    bool config_audio_output(UniqueFramePtr& frame);
    bool config(int sample_rate,
        int channels,
        SDL_AudioFormat format, int first_pts,
        int samples = 1024);
    double get_clock();
private:
    void sdl_callback(void* userdata, Uint8* stream, int len);
    void callback(Uint8* stream, int len);

    int                  m_Width    = 0;
    int                  m_Height   = 0;
    UniqueWindowPtr      m_Window   = nullptr;
    UniqueRenderPtr      m_Renderer = nullptr;
    UniqueTexturePtr     m_Texture  = nullptr;

    // Audio
    
    std::once_flag      m_OnceFlag{};
    SDL_AudioDeviceID   m_DeviceId{0};
    SDL_AudioSpec       m_Spec{};
    int                 m_FirstPts{};
    int                 m_SampleRate{};
    int                 m_Sample{};
    int64_t             m_TotalSamplePlayed{};
    std::deque<uint8_t> m_Deque;
    std::mutex          m_Mutex;
    Clock               m_Clock{};
};

#endif /* _VIEW */