#ifndef _AUDIO_OUTPUT_
#define _AUDIO_OUTPUT_

#include <SDL2/SDL.h>
#include <deque>
#include <mutex>
#include <cstdint>

class audiooutput
{
public:
    audiooutput();
    ~audiooutput();

    bool config(int sample_rate,
                int channels,
                SDL_AudioFormat format,
                int samples = 1024);

    void start();
    void stop();

    // Push PCM data (interleaved)
    void push(const uint8_t* data, size_t size);

    void clear();

private:
    static void sdl_callback(void* userdata, Uint8* stream, int len);
    void callback(Uint8* stream, int len);

    SDL_AudioDeviceID device_id_{0};
    SDL_AudioSpec spec_{};

    std::deque<uint8_t> queue_;
    std::mutex mutex_;
};

#endif /* _AUDIO_OUTPUT_*/