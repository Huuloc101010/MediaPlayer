#include <iostream>
#include <unordered_map>
#include "Player.h"
#include "VideoOutput.h"
#include "AudioOutput.h"
#include "Log.h"

Player::Player() : m_TheadProcessEvent(&Player::TheadProcessEvent, this)
{
    m_MapProcessing =
    {
        {PlayerEvent::QUIT,    [this](){EventQuit();}},
        {PlayerEvent::STOP,    [this](){EventStop();}},
        {PlayerEvent::NEXT,    [this](){EventNext();}},
        {PlayerEvent::PRIVIOUS,[this](){EventPrivious();}},
        {PlayerEvent::PAUSE,   [this](){EventPause();}},
        {PlayerEvent::PLAY,    [this](){EventPlay();}},
        {PlayerEvent::SEEK,    [this](){EventSeek();}},
    };

    m_Controller   = std::make_unique<Controller>(this);
    m_View         = std::make_unique<View>(this);
}

void Player::EventQuit()
{
    LOGW("Received event quit");
    exit(0);
}

void Player::EventStop()
{
    switch(m_PlayerState.load())
    {

        case PlayerState::PLAYING:
        case PlayerState::PAUSED:
        {
            if(m_Demuxer)      m_Demuxer     ->Stop();
            if(m_VideoOutput)  m_VideoOutput ->Stop();
            if(m_AudioOutput)  m_AudioOutput ->Stop();
            if(m_VideoDecoder) m_VideoDecoder->Stop();
            if(m_AudioDecoder) m_AudioDecoder->Stop();
            if(m_View)         m_View        ->Stop();
            if(m_Controller)   m_Controller  ->Stop();
            break;
        }
        default:
        {
            LOGW("This event can't process in current state");
            break;
        }
    }
    m_PlayerState = PlayerState::STOPPED;
    LOGD("Received event stop");
}

void Player::EventNext()
{
    m_PlayList.Next();
    PlayOtherMedia();
}

void Player::EventPrivious()
{
    m_PlayList.Privious();
    PlayOtherMedia();
}

void Player::PlayOtherMedia()
{
    switch(m_PlayerState.load())
    {
        case PlayerState::IDLE:
        case PlayerState::STOPPED:
        case PlayerState::PLAYING:
        case PlayerState::PAUSED:
        case PlayerState::EXITING:
        {
            m_PlayerState = PlayerState::STOPPED;
            if(m_Demuxer)      m_Demuxer      ->Stop();
            if(m_VideoDecoder) m_VideoDecoder ->Stop();
            if(m_AudioDecoder) m_AudioDecoder ->Stop();
            if(m_VideoOutput)  m_VideoOutput  ->Stop();
            if(m_AudioOutput)  m_AudioOutput  ->Stop();
            if(m_Controller)   m_Controller   ->Stop();

            m_PlayerState = PlayerState::EXITING;
            if(m_VideoOutput)  m_VideoOutput  ->Exit();
            if(m_AudioOutput)  m_AudioOutput  ->Exit();
            if(m_AudioDecoder) m_AudioDecoder ->Exit();
            if(m_VideoDecoder) m_VideoDecoder ->Exit();
            if(m_Demuxer)      m_Demuxer      ->Exit();
            if(m_View)         m_View         ->Exit();
            m_PlayerState = PlayerState::IDLE;
            m_PlayerEvent.Clear();
            Player::Start();
            break;
        }
        default:
        {
            LOGW("This event can't process in current state");
            break;
        }
    }
    LOGD("Received event next");
}

void Player::EventPause()
{
    switch(m_PlayerState.load())
    {

        case PlayerState::PLAYING:
        {
            if(m_Demuxer)      m_Demuxer      ->Pause();
            if(m_VideoOutput)  m_VideoOutput  ->Pause();
            if(m_AudioOutput)  m_AudioOutput  ->Pause();
            if(m_VideoDecoder) m_VideoDecoder ->Pause();
            if(m_AudioDecoder) m_AudioDecoder ->Pause();
            if(m_View)         m_View         ->Pause();
            if(m_Controller)   m_Controller   ->Pause();
            break;
        }
        default:
        {
            LOGW("This event can't process in current state");
            break;
        }
    }
    m_PlayerState = PlayerState::PAUSED;
    LOGD("Received event pause");
}

void Player::EventPlay()
{
    switch(m_PlayerState.load())
    {
        case PlayerState::PAUSED:
        case PlayerState::IDLE:
        {
            if(m_Demuxer)      m_Demuxer     ->Play();
            if(m_VideoOutput)  m_VideoOutput ->Play();
            if(m_AudioOutput)  m_AudioOutput ->Play();
            if(m_VideoDecoder) m_VideoDecoder->Play();
            if(m_AudioDecoder) m_AudioDecoder->Play();
            if(m_View)         m_View        ->Play();
            if(m_Controller)   m_Controller  ->Play();
            break;
        }
        default:
        {
            LOGW("This event can't process in current state");
            break;
        }

    }
    m_PlayerState = PlayerState::PLAYING;
    LOGD("Received event play");
}

void Player::EventSeek()
{
    LOGE("Seek called");
}

void Player::PushEvent(PlayerEvent Event)
{
    m_PlayerEvent.Push(Event);
}

void Player::TheadProcessEvent()
{
    while(true)
    {
        auto EventOpt = m_PlayerEvent.Pop();
        if(EventOpt == std::nullopt)
        {
            LOGW("Event null Opt");
            continue;
        }
        PlayerEvent Event = EventOpt.value();
        LOGI("Received event {}", static_cast<int>(Event));
        if(m_MapProcessing.at(Event))
        {
            m_MapProcessing.at(Event)();
        }
    }
}

void Player::MainThreadProcess()
{
    while(true)
    {
        m_View->ShowVideo();
        m_View->CheckResizeWindow();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Player::Config(const std::string& MediaDirectory)
{
    // Scan list media in directory
    m_PlayList.Scan(MediaDirectory);
}

std::string Player::err2str(int errnum)
{
    std::string buf(AV_ERROR_MAX_STRING_SIZE, '\0');
    av_strerror(errnum, buf.data(), buf.size());
    return buf;
}

std::string Player::ts2timestr(int64_t ts, AVRational tb)
{
    std::string buf(AV_TS_MAX_STRING_SIZE, '\0');
    av_ts_make_time_string(buf.data(), ts, &tb);
    return buf;
}

bool Player::PushVideoFrameToView(UniqueFramePtr frame)
{
    if(m_View == nullptr)
    {
        LOGE("m_View is nullptr");
        return false;
    }
    m_View->PushVideoFrame(std::move(frame));
    return true;
}

int Player::OutputVideoFrame(UniqueFramePtr frame)
{
    if(frame == nullptr)
    {
        LOGE("frame is null");
        return -1;
    }
    if(m_VideoOutput)
    {
        m_VideoOutput->PushQueue(std::move(frame));
    }
    else
    {
        LOGE("video output is nullptr");
        return -1;
    }
    return 0;
}

int Player::OutputAudioFrame(UniqueFramePtr frame)
{
    if(m_AudioOutput)
    {
        m_AudioOutput->PushQueue(std::move(frame));
    }
    else
    {
        LOGE("m_AudioOutput is null");
    }
    return 0;
}

int Player::Start()
{
    m_Demuxer      = std::make_unique<Demuxer>(this);
    m_VideoDecoder = std::make_unique<VideoDecoder>(this);
    m_AudioDecoder = std::make_unique<AudioDecoder>(this);
    m_VideoOutput  = std::make_unique<VideoOutput>(this);
    m_AudioOutput  = std::make_unique<AudioOutput>(this);

    LOGI("Create new object success");
    int Ret = -1;
    if(m_Demuxer != nullptr)
    {
        Ret = m_Demuxer->StartPlay(m_PlayList.GetCurrentMedia());
    }
    
    LOGI("Demuxing succeeded");
    return Ret;
}

bool Player::ConfigVideoOutput()
{
    /* allocate image where the decoded image will be put */
    if(m_VideoOutput && m_VideoDecoder && m_View)
    {
        if(!m_VideoOutput->StartThread())
        {
            LOGE("start thread fail");
            return false;
        }
    }
    else
    {
        LOGE("m_VideoOutput = nullptr");
        return false;
    }
    return true;
}

int Player::GetVideoWidth()
{
    if(m_VideoDecoder == nullptr)
    {
        LOGE("Video Decoder is nullptr");
        return {};
    }
    return m_VideoDecoder->GetWidth();
}

int Player::GetVideoHeight()
{
    if(m_VideoDecoder == nullptr)
    {
        LOGE("Video Decoder is nullptr");
        return {};
    }
    return m_VideoDecoder->GetHeight();
}

bool Player::ConfigAudioOutput()
{
    if(m_AudioOutput)
    {
        if(!m_AudioOutput->StartThread())
        {
            LOGE("start thread fail");
            return false;
        }
    }
    return true;
}

int Player::DecodePacket(UniquePacketPtr pkt, const bool IsFlushDecoder)
{
    if((pkt == nullptr) && (IsFlushDecoder == false))
    {
        LOGE("packet is nullptr");
        return -1;
    }
    if(m_Demuxer == nullptr)
    {
        LOGE("m_Demuxer is nullptr");
        return -1;
    }
    if(IsFlushDecoder)
    {
        /* flush the decoders */
        if(m_VideoDecoder) m_VideoDecoder->PushPacket(nullptr);
        if(m_AudioDecoder) m_AudioDecoder->PushPacket(nullptr);
        return 0;
    }
    if((pkt->stream_index == m_Demuxer->GetVideoStreamIndex()) && (m_VideoDecoder))
    {
        m_VideoDecoder->PushPacket(std::move(pkt));
    }
    else if((pkt->stream_index == m_Demuxer->GetAudioStreamIndex()) && (m_AudioDecoder))
    {
        m_AudioDecoder->PushPacket(std::move(pkt));
    }
    return 0;
}

double Player::GetAudioClock()
{
    double audioclock{};
    if(m_View != nullptr)
    {
        audioclock = m_View->GetClock();
    }
    return audioclock;
}

AVRational Player::GetTimeBaseAudio()
{
    if(m_Demuxer == nullptr)
    {
        LOGE("Demuxer is nullptr");
        return {};
    }
    return m_Demuxer->GetTimeBaseAudio();
}

AVRational Player::GetTimeBaseVideo()
{
    if(m_Demuxer == nullptr)
    {
        LOGE("Demuxer is nullptr");
        return {};
    }
    return m_Demuxer->GetTimeBaseVideo();
}

std::atomic<PlayerState>& Player::GetCurrentState()
{
    return m_PlayerState;
}

bool Player::InitVideoDecoder(const AVCodecID codecID, AVCodecParameters* codec_par)
{
    if(m_VideoDecoder && (m_VideoDecoder->ConfigDecoder(codecID, codec_par) != 0))
    {
        LOGE("Init Video Decoder fail");
        return false;
    }
    return true;
}

bool Player::InitAudioDecoder(const AVCodecID codecID, AVCodecParameters* codec_par)
{
    if(m_AudioDecoder && (m_AudioDecoder->ConfigDecoder(codecID, codec_par) != 0))
    {
        LOGE("Init Audio Decoder fail");
        return false;
    }
    return true;
}

void Player::PushSDLAudioData(const uint8_t* data, size_t Size)
{
    if(m_View == nullptr)
    {
        LOGE("View ptr is null");
    }
    m_View->Push(data, Size);
}

bool Player::AudioConfig(int sample_rate,
                    int channels,
                    SDL_AudioFormat format,
                    int first_pts,
                    int samples)
{
    if(m_View == nullptr)
    {
        LOGE("View is nullptr");
        return false;
    }
    return m_View->Config(sample_rate, channels, format, first_pts, samples);
}

void Player::VideoConfig(int width, int height)
{
    if(m_View == nullptr)
    {
        LOGE("View ptr is null");
        return;
    }
    m_View->Config({height, width});
}

Rect Player::CheckInWhichButton(const Position postion)
{
    if(m_View == nullptr)
    {
        LOGE("View ptr is null");
        return Rect::NONE;
    }
    return m_View->CheckInWhichButton(postion);
}

double Player::GetTotalVideoTime()
{
    if(m_Demuxer == nullptr)
    {
        LOGE("View ptr is null");
        return 0.0;
    }
    return m_Demuxer->GetTotalVideoTime();
}