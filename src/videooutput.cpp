#include <vector>
#include "videooutput.h"
#include "log.h"
#include "mediator.h"

videooutput::videooutput(mediator* mediator) : m_Mediator(mediator)
{
    SetLimitQueueOutput(LIMIT_QUEUE_VIDEO_FRAME);
}

videooutput::~videooutput()
{
    if(m_ThreadShow.joinable())
    {
        m_ThreadShow.join();
    }
}

void videooutput::Play()
{
    controlfunction::Play();
}
void videooutput::Pause()
{
    controlfunction::Pause();
}

void videooutput::Stop()
{
    controlfunction::Stop();
}

void videooutput::Exit()
{
    controlfunction::Exit();
    m_QueueSafe.release();
    output::Exit();
}

bool videooutput::ConvertFramePtrToRawData(UniqueFramePtr frame)
{
    if(frame == nullptr)
    {
        LOGE("fail to show video frame");
        return false;
    }
    if(frame->format == AV_PIX_FMT_YUV420P)
    {
        // double pts = frame->best_effort_timestamp * av_q2d(m_video_stream->time_base);
        // LOGI("video pts:{}", pts);
        yuv lyuv =
        {
            frame->data[0],
            frame->data[1],
            frame->data[2],
            frame->linesize[0],
            frame->linesize[1],
            frame->linesize[2]
        };
        
        if(m_Mediator)
        {
            m_Mediator->UpdateYUVTexture(lyuv);
        }
    }
    return 0;
}

void videooutput::ThreadProcessFramePtr()
{
    if(m_Mediator == nullptr)
    {
        LOGE("m_Mediator is null");
        return;
    }
    if(m_Mediator->InitView() == false)
    {
        return;
    }

    while(m_PlayerState != PlayerState::EXITING)
    {

        if(CheckStateExit())
        {
            return;
        }
        CheckStateSleep();
        
        auto retval = std::move(m_QueueSafe.pop());
        if(retval == std::nullopt)
        {
            //LOGW("nullopt");
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
        ConvertFramePtrToRawData(std::move(FramePtr));
    }
}
