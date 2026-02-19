#ifndef _AUDIO_OUTPUT_
#define _AUDIO_OUTPUT_

#include <SDL2/SDL.h>
#include <deque>
#include <mutex>
#include <cstdint>
#include <thread>
#include "define.h"
#include "output.h"

class mediator;

class audiooutput : public output
{
public:
    audiooutput(mediator* mediator);
    ~audiooutput();

    bool config(int sample_rate,
                int channels,
                SDL_AudioFormat format, int first_pts,
                int samples = 1024);

    void start() override;
    void stop() override;
    const double get_clock() override;
    // Push PCM data (interleaved)
    void push(const uint8_t* data, size_t size);

    void clear() override;
    void audio_convert(UniqueFramePtr FramePtr);

private:
    static void sdl_callback(void* userdata, Uint8* stream, int len);
    void callback(Uint8* stream, int len);
    bool config_audio_output(UniqueFramePtr& frame);
    void ThreadProcessFramePtr() override;

    SwrContext*         m_SwrContext = nullptr;
    std::once_flag      m_OnceFlag{};
    mediator*           m_Mediator{};
    SDL_AudioDeviceID   m_DeviceId{0};
    SDL_AudioSpec       m_Spec{};
    int                 m_FirstPts{};
    int                 m_SampleRate{};
    int                 m_Sample{};
    int64_t             m_TotalSamplePlayed{};
    std::deque<uint8_t> m_Deque;
    std::mutex          m_Mutex;
};

#endif /* _AUDIO_OUTPUT_*/