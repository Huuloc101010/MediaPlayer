#ifndef _AUDIO_OUTPUT_
#define _AUDIO_OUTPUT_

#include <SDL2/SDL.h>
#include <deque>
#include <mutex>
#include <cstdint>
#include "define.h"

class audiooutput
{
public:
    audiooutput();
    ~audiooutput();

    bool config(int sample_rate,
                int channels,
                SDL_AudioFormat format, int first_pts,
                int samples = 1024);

    void start();
    void stop();

    // Push PCM data (interleaved)
    void push(const uint8_t* data, size_t size);

    void clear();

private:
    static void sdl_callback(void* userdata, Uint8* stream, int len);
    void callback(Uint8* stream, int len);

    SDL_AudioDeviceID   m_DeviceId{0};
    SDL_AudioSpec       m_spec{};
    AudioClock          m_AudioClock{};
    int                 m_first_pts{};
    int                 m_sample_rate{};
    int                 m_sample{};
    int64_t             m_total_samples_played{};
    std::deque<uint8_t> m_deque;
    std::mutex          m_mutex;
};

#endif /* _AUDIO_OUTPUT_*/