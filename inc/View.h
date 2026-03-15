#ifndef _VIEW_
#define _VIEW_

#include <SDL2/SDL.h>
#include "Define.h"
#include "ControlFunction.h"

class View : public ControlFunction
{
public:
    View();
    ~View();
    bool Init();
    void Config(const int Width, const int Height);
    bool UpdateYUVTexture(const yuv& ndata);
    void Play() override;
    void Pause()override;
    void Stop() override;
    void Exit() override;

    void SDLStart();
    void SDLPause();
    void SDLStop();

    void Push(const uint8_t* data, size_t Size);
    void Clear();
    
    bool ConfigAudioOutput(UniqueFramePtr& frame);
    bool Config(int sample_rate,
        int channels,
        SDL_AudioFormat format, int first_pts,
        int samples = 1024);
    double GetClock();
private:
    void SDLCallback(void* userdata, Uint8* stream, int len);
    void Callback(Uint8* stream, int len);

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