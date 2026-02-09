#include "audiooutput.h"
#include <cstring>

audiooutput::audiooutput()
{
    if(SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        exit(1);
    }

}

audiooutput::~audiooutput()
{
    stop();
}

bool audiooutput::config(int sample_rate,
                         int channels,
                         SDL_AudioFormat format,
                         int samples)
{
    SDL_AudioSpec want{};
    want.freq = sample_rate;
    want.channels = channels;
    want.format = format;
    want.samples = samples;
    want.callback = sdl_callback;
    want.userdata = this;

    device_id_ = SDL_OpenAudioDevice(nullptr, 0, &want, &spec_, 0);
    if (!device_id_)
    {
        SDL_Log("SDL_OpenAudioDevice failed: %s", SDL_GetError());
        return false;
    }

    return true;
}

void audiooutput::start()
{
    if(device_id_)
    {
        SDL_PauseAudioDevice(device_id_, 0);
    }
}

void audiooutput::stop()
{
    if(device_id_)
    {
        SDL_PauseAudioDevice(device_id_, 1);
        SDL_CloseAudioDevice(device_id_);
        device_id_ = 0;
    }
}

void audiooutput::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.clear();
}

void audiooutput::push(const uint8_t* data, size_t size)
{
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.insert(queue_.end(), data, data + size);
}

void audiooutput::sdl_callback(void* userdata, Uint8* stream, int len)
{
    static_cast<audiooutput*>(userdata)->callback(stream, len);
}

void audiooutput::callback(Uint8* stream, int len)
{
    std::memset(stream, 0, len); // silence nếu thiếu data

    std::lock_guard<std::mutex> lock(mutex_);

    int to_copy = std::min<int>(len, queue_.size());
    for(int i = 0; i < to_copy; ++i) 
    {
        stream[i] = queue_.front();
        queue_.pop_front();
    }
}
