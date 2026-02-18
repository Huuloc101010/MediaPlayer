#include <vector>
#include "videooutput.h"
#include "log.h"
#include "mediator.h"

videooutput::videooutput(const int width,const int height, mediator* mediator)
: m_ThreadCheckEvent(&videooutput::checkevent, this),
  m_ThreadShow(&videooutput::show3, this),
  m_mediator(mediator)
{
    m_width = width;
    m_height = height;
}

videooutput::~videooutput()
{
    m_exiting = true;
    if(m_ThreadCheckEvent.joinable())
    {
        m_ThreadCheckEvent.join();
    }
    destroy();
}

bool videooutput::init()
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        LOGE("SDL_Init failed: {}", SDL_GetError());
        return false;
    }

    m_window = SDL_CreateWindow(NAME_WINDOW,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_width, m_height, SDL_WINDOW_SHOWN);
    if(m_window == nullptr)
    {
        LOGE("Create window SDL fail: {}", SDL_GetError());
        return false;
    }
    LOGI("Create windows success");
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
    if(m_renderer == nullptr)
    {
        std::cerr << "Create renderer SDL fail: " << SDL_GetError() << std::endl;
        LOGE("Create renderer SDL fail: {}", SDL_GetError());
        return false;
    }
    LOGI("create render success");
    m_texture = SDL_CreateTexture(m_renderer, 
        SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, m_width, m_height);
    if (m_texture == nullptr)
    {
        LOGE("Create texture SDL fail: {}", SDL_GetError());
        return false;
    }
    LOGI("Create texture success ");
    return true;
}

bool videooutput::show(const yuv& ndata)
{
    // if(m_mediator != nullptr)
    // {
    //     LOGE("audio clock:{}", m_mediator->GetAudioClock());
    // }
    // else
    // {
    //     LOGE("Faill");
    // }
    // Push data to GPU
    if(SDL_UpdateYUVTexture(m_texture, NULL, 
        ndata.plane_y, ndata.linesize_y,           
        ndata.plane_u, ndata.linesize_u,       
        ndata.plane_v, ndata.linesize_v) < 0)
    {
        LOGE("update YUV texture fail");
        return false;
    }     

    if(SDL_RenderClear(m_renderer) < 0)
    {
        LOGE("render clear fail");
        return false;
    }
    if(SDL_RenderCopy(m_renderer, m_texture, NULL, NULL) < 0)
    {
        LOGE("Render copy fail");
        return false;
    }
    SDL_RenderPresent(m_renderer);
    return true;
}

void videooutput::destroy()
{
    if (m_texture)
    {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }
    if (m_renderer)
    {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    SDL_Quit();
}

void videooutput::checkevent()
{
    while(!m_exiting)
    {
        while(SDL_PollEvent(&m_event))
        {
            if(m_event.type == SDL_QUIT)
            {
                LOGI("Event SDL_QUIT");
                LOGW("Exitting");
                exit(0);
            }
        }
        // decrease cpu workload
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void videooutput::push_queue(UniqueFramePtr framePtr)
{
    std::lock_guard<std::mutex> lock_guard(m_QueueSafe.mutex);
    m_QueueSafe.queue.emplace_front(std::move(framePtr));
}

bool videooutput::show2(UniqueFramePtr frame)
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
        
        show(lyuv);
    }
    return 0;
}

void videooutput::show3()
{
    init();
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
        double audio_pts = m_mediator->GetAudioClock();
        LOGE("audio clock: {}", audio_pts);

        double video_pts = 0;
        if(FramePtr->best_effort_timestamp != AV_NOPTS_VALUE)
        {
            video_pts = FramePtr->best_effort_timestamp *
                av_q2d(m_mediator->GetTimeBaseVideo());
        }
        LOGW("video clock: {}", video_pts);

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
        show2(std::move(FramePtr));
    }
}