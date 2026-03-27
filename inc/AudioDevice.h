#ifndef AUDIO_DEVICE
#define AUDIO_DEVICE

#include "Define.h"

class AudioDevice
{
public:
    void SDLStart();
    void SDLPause();
    void SDLStop();
    void Clear();
    bool Config(int sample_rate,
                int channels,
                SDL_AudioFormat format,
                int first_pts,
                int samples = 1024);
    std::atomic<double>& GetClock();
    void Push(const uint8_t* data, size_t Size);
    void ClearAudioPts();
    void SetClockBase(double time);
private:

    void Callback(Uint8* stream, int len);

    std::once_flag      m_OnceFlag{};
    SDL_AudioDeviceID   m_DeviceId{0};
    SDL_AudioSpec       m_Spec{};
    int                 m_FirstPts{};
    int                 m_SampleRate{};
    int                 m_Sample{};
    int                 m_Channel;
    int64_t             m_TotalSamplePlayed{};
    std::deque<uint8_t> m_Deque;
    std::mutex          m_Mutex;
    Clock               m_Clock{};
};

#endif /* AUDIO_DEVICE */