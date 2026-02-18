#include "audiooutput.h"
#include "log.h"
#include <cstring>
#include "mediator.h"

audiooutput::audiooutput(mediator* mediator)
                        : m_mediator(mediator)
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
    swr_free(&m_swr);
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
    std::memset(stream, 0, len); // silence if not enable data

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
    //LOGW("audio clock = {}", m_AudioClock.pts.load());
}
const double audiooutput::get_clock()
{
    return m_AudioClock.pts;
}

void audiooutput::audio_convert(UniqueFramePtr FramePtr)
{
    if(FramePtr == nullptr)
    {
        LOGE("FramePtr is nullptr");
        return;
    }
    size_t unpadded_linesize = FramePtr->nb_samples * av_get_bytes_per_sample((AVSampleFormat)FramePtr->format);
 
    //double pts = m_frame->best_effort_timestamp * av_q2d(m_audio_stream->time_base);
    //LOGI("Audio pts:{}", pts);
//    LOGI("Audio frame->nb_samples={}", frame->nb_samples);
//    LOGE("{}", frame->pts);
    std::call_once(m_once_flag,
    [&](void)->void
    {
        if(!config_audio_output(FramePtr))
        {
            LOGE("config audio fail");
            return;
        }
        else
        {
            LOGI("config audio success");
        }
    });

    int out_samples = av_rescale_rnd(
        swr_get_delay(m_swr, FramePtr->sample_rate) + FramePtr->nb_samples,
        FramePtr->sample_rate,
        FramePtr->sample_rate,
        AV_ROUND_UP
    );

    /* allocate buffer output */
    int out_linesize = 0;
    AudioS16Buffer out{};
    uint64_t ch_layout =
        FramePtr->channel_layout ?
        FramePtr->channel_layout :
        av_get_default_channel_layout(FramePtr->channels);
    int out_channels = av_get_channel_layout_nb_channels(ch_layout);
    av_samples_alloc(
        &out.data,
        &out_linesize,
        out_channels,
        out_samples,
        AV_SAMPLE_FMT_S16,
        0
    );

    /*  Convert */
    int samples = swr_convert(
        m_swr,
        &out.data,
        out_samples,
        (const uint8_t**)FramePtr->data,
        FramePtr->nb_samples
    );

    if (samples <= 0)
    {
        av_freep(&out.data);
        LOGE("fail to convert data");
        return;
    }
    out.size = samples * out_channels * sizeof(int16_t);

    if(out.data && out.size > 0)
    {
        push(out.data, out.size);
        av_freep(&out.data);
    }
}

bool audiooutput::config_audio_output(UniqueFramePtr& m_frame)
{
    if(m_frame == nullptr)
    {
        return false;
    }
    if(m_mediator == nullptr)
    {
        return false;
    }
    double first_pts = m_frame->best_effort_timestamp * av_q2d(m_mediator->GetTimeBaseAudio());
    uint64_t ch_layout =
    m_frame->channel_layout ?
    m_frame->channel_layout :
    av_get_default_channel_layout(m_frame->channels);
   
    if(!config(m_frame->sample_rate,m_frame->channels ,AUDIO_S16SYS, first_pts))
    {
        std::cerr << "config audio error" << std::endl;
        return false;
    }
    start();
    // init software context
    
    m_swr = swr_alloc_set_opts(
    nullptr,
    ch_layout,
    AV_SAMPLE_FMT_S16,
    m_frame->sample_rate,
    m_frame->channel_layout,
    (AVSampleFormat)m_frame->format,
    m_frame->sample_rate,

    0, nullptr);
    if(!m_swr || swr_init(m_swr))
    {
        swr_free(&m_swr);
        LOGE("!m_swr || swr_init(m_swr)");
        return false;
    }
    return true;
}


void audiooutput::thread_process()
{
    while(true)
    {
        m_QueueSafe.mutex.lock();
        while(m_QueueSafe.queue.empty())
        {
            m_QueueSafe.mutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            m_QueueSafe.mutex.lock();
        }
        UniqueFramePtr FramePtr = std::move(m_QueueSafe.queue.back());
        // remove element
        m_QueueSafe.queue.pop_back();
        m_QueueSafe.mutex.unlock();

        audio_convert(std::move(FramePtr));
    }
}