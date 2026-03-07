#include "audiooutput.h"
#include "log.h"
#include <cstring>
#include "mediator.h"

audiooutput::audiooutput(mediator* mediator)
                        : m_Mediator(mediator)
{
    SetLimitQueueOutput(LIMIT_QUEUE_AUDIO_FRAME);
    if(SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        exit(1);
    }

}

audiooutput::~audiooutput()
{
    SDLStop();
    swr_free(&m_SwrContext);
}

bool audiooutput::config(int sample_rate,
                         int channels,
                         SDL_AudioFormat format,
                         int first_pts,
                         int samples)
{
    m_SampleRate = sample_rate;
    m_FirstPts = first_pts;
    m_Sample = samples;
    SDL_AudioSpec want{};
    want.freq = sample_rate;
    want.channels = channels;
    want.format = format;
    want.samples = samples;
    want.callback = sdl_callback;
    want.userdata = this;

    m_DeviceId = SDL_OpenAudioDevice(nullptr, 0, &want, &m_Spec, 0);
    if (!m_DeviceId)
    {
        SDL_Log("SDL_OpenAudioDevice failed: %s", SDL_GetError());
        return false;
    }

    return true;
}

void audiooutput::SDLStart()
{
    if(m_DeviceId)
    {
        SDL_PauseAudioDevice(m_DeviceId, 0);
    }
}

void audiooutput::SDLPause()
{
    if(m_DeviceId)
    {
        SDL_PauseAudioDevice(m_DeviceId, 1);
    }
}

void audiooutput::SDLStop()
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
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Deque.clear();
}

void audiooutput::push(const uint8_t* data, size_t size)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Deque.insert(m_Deque.end(), data, data + size);
}

void audiooutput::sdl_callback(void* userdata, Uint8* stream, int len)
{
    if((userdata == nullptr) || (stream == nullptr))
    {
        return;
    }
    static_cast<audiooutput*>(userdata)->callback(stream, len);
}

void audiooutput::callback(Uint8* stream, int len)
{
    LOGE("Callback called");
    std::memset(stream, 0, len); // silence if not enable data

    std::lock_guard<std::mutex> lock(m_Mutex);

    int to_copy = std::min<int>(len, m_Deque.size());
    for(int i = 0; i < to_copy; ++i) 
    {
        stream[i] = m_Deque.front();
        m_Deque.pop_front();
    }

    /* calculate audio timestamp */

    m_TotalSamplePlayed += m_Sample;
    m_Clock.last_frame_pts.store(m_Clock.pts);
    m_Clock.pts = m_FirstPts + (static_cast<double>(m_TotalSamplePlayed) / m_SampleRate);
    //LOGW("audio clock = {}", m_Clock.pts.load());
}
const double audiooutput::get_clock()
{
    return m_Clock.pts;
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
    std::call_once(m_OnceFlag,
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
        swr_get_delay(m_SwrContext, FramePtr->sample_rate) + FramePtr->nb_samples,
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
        m_SwrContext,
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

bool audiooutput::config_audio_output(UniqueFramePtr& Frame)
{
    if(Frame == nullptr)
    {
        return false;
    }
    if(m_Mediator == nullptr)
    {
        return false;
    }
    double first_pts = Frame->best_effort_timestamp * av_q2d(m_Mediator->GetTimeBaseAudio());
    uint64_t ch_layout =
    Frame->channel_layout ?
    Frame->channel_layout :
    av_get_default_channel_layout(Frame->channels);
   
    if(!config(Frame->sample_rate,Frame->channels ,AUDIO_S16SYS, first_pts))
    {
        std::cerr << "config audio error" << std::endl;
        return false;
    }
    SDLStart();
    // init software context
    
    m_SwrContext = swr_alloc_set_opts(
    nullptr,
    ch_layout,
    AV_SAMPLE_FMT_S16,
    Frame->sample_rate,
    Frame->channel_layout,
    (AVSampleFormat)Frame->format,
    Frame->sample_rate,

    0, nullptr);
    if(!m_SwrContext || swr_init(m_SwrContext))
    {
        swr_free(&m_SwrContext);
        LOGE("!m_SwrContext || swr_init(m_SwrContext)");
        return false;
    }
    return true;
}


void audiooutput::ThreadProcessFramePtr()
{
    while(m_PlayerState.load() != PlayerState::EXITING)
    {
        if(CheckStateExit())
        {
            return;
        }
        CheckStateSleep();

        auto retval = std::move(m_QueueSafe.pop());
        if(retval == std::nullopt)
        {
            LOGW("nullopt");
            continue;
        }
        audio_convert(std::move(retval.value()));
    }
}

void audiooutput::Play()
{
    controlfunction::Play();
    audiooutput::SDLStart();
}

void audiooutput::Pause()
{
    controlfunction::Play();
    audiooutput::SDLPause();
}

void audiooutput::Stop()
{
    controlfunction::Stop();
    audiooutput::SDLStop();
}