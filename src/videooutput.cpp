#include <vector>
#include "videooutput.h"
#include "log.h"
#include "mediator.h"

videooutput::videooutput(mediator* mediator)
            : m_ThreadCheckEvent(&videooutput::checkevent, this)
            , m_Mediator(mediator)
{
}

videooutput::~videooutput()
{
    m_Exiting = true;
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

    m_Window = SDL_CreateWindow(NAME_WINDOW,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_Width, m_Height, SDL_WINDOW_SHOWN);
    if(m_Window == nullptr)
    {
        LOGE("Create window SDL fail: {}", SDL_GetError());
        return false;
    }
    LOGI("Create windows success");
    m_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_ACCELERATED);
    if(m_Renderer == nullptr)
    {
        std::cerr << "Create renderer SDL fail: " << SDL_GetError() << std::endl;
        LOGE("Create renderer SDL fail: {}", SDL_GetError());
        return false;
    }
    LOGI("create render success");
    m_Texture = SDL_CreateTexture(m_Renderer, 
        SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, m_Width, m_Height);
    if (m_Texture == nullptr)
    {
        LOGE("Create texture SDL fail: {}", SDL_GetError());
        return false;
    }
    LOGI("Create texture success ");
    return true;
}

bool videooutput::UpdateYUVTexture(const yuv& ndata)
{
    // Push data to GPU
    if(SDL_UpdateYUVTexture(m_Texture, NULL, 
        ndata.plane_y, ndata.linesize_y,           
        ndata.plane_u, ndata.linesize_u,       
        ndata.plane_v, ndata.linesize_v) < 0)
    {
        LOGE("update YUV texture fail");
        return false;
    }     

    if(SDL_RenderClear(m_Renderer) < 0)
    {
        LOGE("render clear fail");
        return false;
    }
    if(SDL_RenderCopy(m_Renderer, m_Texture, NULL, NULL) < 0)
    {
        LOGE("Render copy fail");
        return false;
    }
    SDL_RenderPresent(m_Renderer);
    return true;
}

void videooutput::destroy()
{
    if (m_Texture)
    {
        SDL_DestroyTexture(m_Texture);
        m_Texture = nullptr;
    }
    if (m_Renderer)
    {
        SDL_DestroyRenderer(m_Renderer);
        m_Renderer = nullptr;
    }
    if (m_Window)
    {
        SDL_DestroyWindow(m_Window);
        m_Window = nullptr;
    }
    SDL_Quit();
}

void videooutput::checkevent()
{
    while(!m_Exiting)
    {
        while(SDL_PollEvent(&m_Event))
        {
            if(m_Event.type == SDL_QUIT)
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
        
        UpdateYUVTexture(lyuv);
    }
    return 0;
}

void videooutput::ThreadProcessFramePtr()
{
    init();
    while(!m_Exiting)
    {
        auto retval = std::move(m_QueueSafe.pop());
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
        ConvertFramePtrToRawData(std::move(FramePtr));
    }
}

void videooutput::Config(const int Width, const int Height)
{
    m_Width = Width;
    m_Height = Height;
}