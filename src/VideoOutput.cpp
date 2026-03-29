#include <vector>
#include "VideoOutput.h"
#include "Log.h"
#include "Mediator.h"

VideoOutput::VideoOutput(Mediator* Mediator) : m_Mediator(Mediator)
{
    SetLimitQueueOutput(LIMIT_QUEUE_VIDEO_FRAME);
}

VideoOutput::~VideoOutput()
{
    if(m_ThreadShow.joinable())
    {
        m_ThreadShow.join();
    }
}

void VideoOutput::Play()
{
    ControlFunction::Play();
}
void VideoOutput::Pause()
{
    ControlFunction::Pause();
}

void VideoOutput::Stop()
{
    ControlFunction::Stop();
}

void VideoOutput::Exit()
{
    ControlFunction::Exit();
    m_QueueSafe.Release();
    Output::Exit();
}

bool VideoOutput::PushFrameToView(UniqueFramePtr frame)
{
    if(frame == nullptr)
    {
        LOGE("fail to show video frame");
        return false;
    }
    if(frame->format == AV_PIX_FMT_YUV420P)
    {
        if(m_Mediator)
        {
            m_Mediator->PushVideoFrameToView(std::move(frame));
        }
    }
    return 0;
}

void VideoOutput::ThreadProcessFramePtr()
{
    if(m_Mediator == nullptr)
    {
        LOGE("m_Mediator is null");
        return;
    }
    m_Mediator->VideoConfig(m_Mediator->GetVideoWidth(), m_Mediator->GetVideoHeight());

    while((m_PlayerState != PlayerState::EXITING) && (m_PlayerState != PlayerState::STOPPED))
    {

        if(CheckStateExit())
        {
            return;
        }
        CheckStateSleep();
        auto retval = m_QueueSafe.Pop();
        if(retval == std::nullopt)
        {
            LOGW("nullopt");
            continue;
        }
        UniqueFramePtr FramePtr = std::move(retval.value());
        double audio_pts = m_Mediator->GetAudioClock();
        LOGE("audio clock: {}", audio_pts);

        double video_pts = 0;
        if(FramePtr->best_effort_timestamp != AV_NOPTS_VALUE)
        {
            video_pts = FramePtr->best_effort_timestamp *
                av_q2d(m_Mediator->GetTimeBaseVideo());
        }
        m_Clock.last_frame_pts.store(m_Clock.pts.load());
        m_Clock.pts = video_pts;
        LOGW("video clock: {}", m_Clock.pts.load());

        double diff = video_pts - audio_pts;

        // video > audio 0.04s -> sleep
        if(diff > 0)
        {
            std::this_thread::sleep_for(std::chrono::duration<double>(diff));
        }
        // video < audio -> skip frame
        if((video_pts - audio_pts) < -0.04)
        {
            LOGW("Skip frame");
            continue;
        }

        // video - audio = ( -0.04 -> 0) show frame
        PushFrameToView(std::move(FramePtr));
    }
}
