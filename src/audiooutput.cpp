#include "audiooutput.h"
#include "log.h"
#include <cstring>
#include "mediator.h"

audiooutput::audiooutput(mediator* mediator) : m_mediator(mediator)
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
                         int first_pts,
                         int samples)
{
    m_sample_rate = sample_rate;
    m_first_pts = first_pts;
    m_sample = samples;
    SDL_AudioSpec want{};
    want.freq = sample_rate;
    want.channels = channels;
    want.format = format;
    want.samples = samples;
    want.callback = sdl_callback;
    want.userdata = this;

    m_DeviceId = SDL_OpenAudioDevice(nullptr, 0, &want, &m_spec, 0);
    if (!m_DeviceId)
    {
        SDL_Log("SDL_OpenAudioDevice failed: %s", SDL_GetError());
        return false;
    }

    return true;
}

void audiooutput::start()
{
    if(m_DeviceId)
    {
        SDL_PauseAudioDevice(m_DeviceId, 0);
    }
}

void audiooutput::stop()
{
    if(m_DeviceId)
    {
        SDL_PauseAudioDevice(m_DeviceId, 1);
        SDL_CloseAudioDevice(m_DeviceId);
        m_DeviceId = 0;
    }
}

void audiooutput::clear()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_deque.clear();
}

void audiooutput::push(const uint8_t* data, size_t size)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_deque.insert(m_deque.end(), data, data + size);
}

void audiooutput::sdl_callback(void* userdata, Uint8* stream, int len)
{
    static_cast<audiooutput*>(userdata)->callback(stream, len);
}

void audiooutput::callback(Uint8* stream, int len)
{
    std::memset(stream, 0, len); // silence nếu thiếu data

    std::lock_guard<std::mutex> lock(m_mutex);

    int to_copy = std::min<int>(len, m_deque.size());
    for(int i = 0; i < to_copy; ++i) 
    {
        stream[i] = m_deque.front();
        m_deque.pop_front();
    }

    /* calculate audio timestamp */

    m_total_samples_played += m_sample;
    m_AudioClock.last_frame_pts.store(m_AudioClock.pts);
    m_AudioClock.pts = m_first_pts + (static_cast<double>(m_total_samples_played) / m_sample_rate);
    LOGW("audio clock = {}", m_AudioClock.pts.load());
}
const double audiooutput::get_clock()
{
    return m_AudioClock.pts;
}