#include <cstring>
#include "AudioDevice.h"

void AudioDevice::SDLStart()
{
    if(m_DeviceId)
    {
        SDL_PauseAudioDevice(m_DeviceId, 0);
    }
    else
    {
        LOGE("m_DeviceId = 0");
    }
}

void AudioDevice::SDLPause()
{
    if(m_DeviceId)
    {
        SDL_PauseAudioDevice(m_DeviceId, 1);
    }
    else
    {
        LOGE("m_DeviceId = 0");
    }
}

void AudioDevice::SDLStop()
{
    if(m_DeviceId)
    {
        SDL_PauseAudioDevice(m_DeviceId, 1);
    }
    else
    {
        LOGE("m_DeviceId = 0");
    }
}

void AudioDevice::Clear()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    SDL_LockAudioDevice(m_DeviceId);
    m_Deque.clear();
    SDL_ClearQueuedAudio(m_DeviceId); 
    SDL_UnlockAudioDevice(m_DeviceId);
}

bool AudioDevice::Config(int sample_rate,
                         int channels,
                         SDL_AudioFormat format,
                         int first_pts,
                         int samples)
{
    if(m_DeviceId)
    {
        SDL_CloseAudioDevice(m_DeviceId);
        m_DeviceId = 0;
    }
    m_SampleRate = sample_rate;
    m_FirstPts = first_pts;
    m_Sample = samples;
    m_Channel = channels;
    LOGD("m_SampleRate {}", m_SampleRate);
    LOGD("m_FirstPts {}", m_FirstPts);
    LOGD("m_Sample {}", m_Sample);
    LOGD("m_Channel {}", m_Channel);
    // Reset member value
    m_TotalSamplePlayed = 0;
    m_Clock.pts = 0.0;
    m_Clock.last_frame_pts = 0.0;
    SDL_AudioSpec want{};
    want.freq = sample_rate;
    want.channels = channels;
    want.format = format;
    want.samples = samples;
    want.callback = [](void* userdata, Uint8* stream, int len)
    {static_cast<AudioDevice*>(userdata)->Callback(stream, len);};
    want.userdata = this;
    m_DeviceId = SDL_OpenAudioDevice(nullptr, 0, &want, &m_Spec, 0);
    LOGI("Open audio device success: {}", m_DeviceId);
    if(!m_DeviceId)
    {
        LOGE("SDL_OpenAudioDevice failed: {}", SDL_GetError());
        return false;
    }
    return true;
}

void AudioDevice::ClearAudioPts()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Clock.pts = m_FirstPts;
    m_Clock.last_frame_pts = m_FirstPts;
    m_TotalSamplePlayed = 0;
}

void AudioDevice::Callback(Uint8* stream, int len)
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
    int bytes_per_sample = SDL_AUDIO_BITSIZE(m_Spec.format) / 8;
    int samples = to_copy / (bytes_per_sample * m_Spec.channels);

    m_TotalSamplePlayed += samples;
    m_Clock.last_frame_pts.store(m_Clock.pts);
    m_Clock.pts = m_FirstPts + (static_cast<double>(m_TotalSamplePlayed) / m_SampleRate);
    //LOGW("audio clock = {}", m_Clock.pts.load());
}

std::atomic<double>& AudioDevice::GetClock()
{
    return m_Clock.pts;
}

void AudioDevice::Push(const uint8_t* data, size_t Size)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Deque.insert(m_Deque.end(), data, data + Size);
}

void AudioDevice::SetClockBase(double time)
{
    m_FirstPts = time;
    m_TotalSamplePlayed = 0;
    m_Clock.pts = time;
    m_Clock.last_frame_pts = time;
}
