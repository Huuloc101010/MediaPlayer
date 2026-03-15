#include "audiooutput.h"
#include "log.h"
#include <cstring>
#include "mediator.h"

audiooutput::audiooutput(mediator* mediator)
                        : m_Mediator(mediator)
{
    SetLimitQueueOutput(LIMIT_QUEUE_AUDIO_FRAME);
}

audiooutput::~audiooutput()
{
    if(m_ThreadShow.joinable())
    {
        m_ThreadShow.join();
    }
    swr_free(&m_SwrContext);
}

double audiooutput::get_clock()
{
    return 0;//m_Clock.pts;
}

void audiooutput::audio_convert(UniqueFramePtr FramePtr)
{
    if(FramePtr == nullptr)
    {
        LOGE("FramePtr is nullptr");
        return;
    }
 
    //LOGE("{}", frame->pts);
    std::call_once(m_OnceFlag,
    [&](void)->void
    {
        if(config_audio_output(FramePtr) == false)
        {
            LOGE("config audio fail");
            return;
        }
        else
        {
            LOGI("config audio success");
        }
    });
    if(m_SwrContext == nullptr)
    {
        LOGE("m_SwrContext is null");
        return;
    }
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
        if(m_Mediator == nullptr)
        {
            LOGE("Mediator is null");
            return;
        }
        m_Mediator->PushSDLAudioData(out.data, out.size);
        av_freep(&out.data);
    }
}

bool audiooutput::config_audio_output(UniqueFramePtr& Frame)
{
    if(Frame == nullptr)
    {
        LOGE("Frame is null");
        return false;
    }
    if(m_Mediator == nullptr)
    {
        LOGE("Mediator is null");
        return false;
    }
    double first_pts = Frame->best_effort_timestamp * av_q2d(m_Mediator->GetTimeBaseAudio());
    uint64_t ch_layout =
    Frame->channel_layout ?
    Frame->channel_layout :
    av_get_default_channel_layout(Frame->channels);
   
    if(m_Mediator->AudioConfig(Frame->sample_rate,Frame->channels ,AUDIO_S16SYS, first_pts) == false)
    {
        LOGE("config audio error");
        return false;
    }
    
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
    while((m_PlayerState.load() != PlayerState::EXITING) && (m_PlayerState.load() != PlayerState::EXITING))
    {
        if(CheckStateExit())
        {
            return;
        }
        CheckStateSleep();

        auto retval = m_QueueSafe.pop();
        if(retval == std::nullopt)
        {
            LOGW("nullopt");
            continue;
        }
        audio_convert(std::move(retval.value()));
    }
}
