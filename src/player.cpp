#include <iostream>
#include <unordered_map>
#include "player.h"
#include "videooutput.h"
#include "audiooutput.h"
#include "log.h"

player::player() : m_TheadProcessEvent(&player::TheadProcessEvent, this)
{
    m_MapProcessing =
    {
        {PlayerEvent::QUIT,  [this](){EventQuit();}},
        {PlayerEvent::STOP,  [this](){EventStop();}},
        {PlayerEvent::NEXT,  [this](){EventNext();}},
        {PlayerEvent::PAUSE, [this](){EventPause();}},
        {PlayerEvent::PLAY,  [this](){EventPlay();}},
    };
}

void player::EventQuit()
{
    LOGW("Received event quit");
    LOGW("Received event quit");
    LOGW("Received event quit");
    LOGW("Received event quit");
    LOGW("Received event quit");
    exit(0);
}

void player::EventStop()
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
            break;
        }
        default:
        {
            LOGW("This event can't process in current state");
            break;
        }
    }
    m_PlayerState = PlayerState::STOPPED;
    LOGW("Received event stop");
}

void player::EventNext()
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
            LOGW("1");
            if(m_Controller)   m_Controller   ->Stop();
            LOGW("2");
            if(m_Demuxer)      m_Demuxer      ->Stop();
            LOGW("3");
            if(m_VideoDecoder) m_VideoDecoder ->Stop();
            LOGW("4");
            if(m_AudioDecoder) m_AudioDecoder ->Stop();
            LOGW("5");
            if(m_VideoOutput)  m_VideoOutput  ->Stop();
            LOGW("6");
            if(m_AudioOutput)  m_AudioOutput  ->Stop();
            LOGW("7");
            if(m_View)         m_View         ->Stop();
            LOGW("8");

            m_PlayerState = PlayerState::EXITING;
            LOGW("9");
            if(m_Controller)   m_Controller   ->Exit();
            LOGW("10");
            if(m_View)         m_View         ->Exit();
            LOGW("11");
            if(m_VideoOutput)  m_VideoOutput  ->Exit();
            LOGW("12");
            if(m_AudioOutput)  m_AudioOutput  ->Exit();
            LOGW("13");
            if(m_AudioDecoder) m_AudioDecoder ->Exit();
            LOGW("14");
            if(m_VideoDecoder) m_VideoDecoder ->Exit();
            LOGW("15");
            if(m_Demuxer)      m_Demuxer      ->Exit();
            LOGW("16");
            m_PlayerState = PlayerState::IDLE;
            m_PlayerEvent.clear();
            player::Start();
            break;
        }
        default:
        {
            LOGW("This event can't process in current state");
            break;
        }
    }
    LOGW("Received event next");
}

void player::EventPause()
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
            break;
        }
        default:
        {
            LOGW("This event can't process in current state");
            break;
        }
    }
    m_PlayerState = PlayerState::PAUSED;
    LOGW("Received event pause");
}

void player::EventPlay()
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
            break;
        }
        default:
        {
            LOGW("This event can't process in current state");
            break;
        }

    }
    m_PlayerState = PlayerState::PLAYING;
    LOGW("Received event play");
}

void player::PushEvent(PlayerEvent Event)
{
    m_PlayerEvent.push(Event);
}

void player::TheadProcessEvent()
{
    while(true)
    {
        auto EventOpt = m_PlayerEvent.pop();
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

void player::Config(const std::string& MediaFile)
{
    m_CurrentMedia = MediaFile;
}

std::string player::err2str(int errnum)
{
    std::string buf(AV_ERROR_MAX_STRING_SIZE, '\0');
    av_strerror(errnum, buf.data(), buf.size());
    return buf;
}

std::string player::ts2timestr(int64_t ts, AVRational tb)
{
    std::string buf(AV_TS_MAX_STRING_SIZE, '\0');
    av_ts_make_time_string(buf.data(), ts, &tb);
    return buf;
}

bool player::InitView()
{
    bool RetVal = false;
    if(m_View)
    {
        RetVal = m_View->init();
    }
    if(RetVal == false)
    {
        return false;
    }
    return true;
}

bool player::UpdateYUVTexture(const yuv& ndata)
{
    bool RetVal = false;
    if(m_View)
    {
        RetVal = m_View->UpdateYUVTexture(ndata);
    }
    return RetVal;
}

int player::output_video_frame(UniqueFramePtr frame)
{
    if(frame == nullptr)
    {
        LOGE("frame is null");
        return -1;
    }
    if(m_VideoOutput)
    {
        m_VideoOutput->push_queue(std::move(frame));
    }
    else
    {
        LOGE("video output is nullptr");
        return -1;
    }
    return 0;
}

int player::output_audio_frame(UniqueFramePtr frame)
{
    if(m_AudioOutput)
    {
        m_AudioOutput->push_queue(std::move(frame));
    }
    else
    {
        LOGE("m_AudioOutput is null");
    }
    return 0;
}

int player::Start()
{
    LOGE("1");
    m_Demuxer      = std::make_unique<demuxer>(this);
    LOGE("2");
    m_VideoDecoder = std::make_unique<videodecoder>(this);
    LOGE("3");
    m_AudioDecoder = std::make_unique<audiodecoder>(this);
    LOGE("4");
    m_VideoOutput  = std::make_unique<videooutput>(this);
    LOGE("5");
    m_AudioOutput  = std::make_unique<audiooutput>(this);
    LOGE("6");
    m_Controller   = std::make_unique<controller>(this);
    LOGE("7");
    m_View         = std::make_unique<view>();
    LOGI("Create new object success");
    int Ret = -1;
    if(m_Demuxer != nullptr)
    {
        Ret = m_Demuxer->StartPlay(m_CurrentMedia);
    }
    
    LOGI("Demuxing succeeded");
    return Ret;
}

bool player::ConfigVideoOutput()
{
    /* allocate image where the decoded image will be put */
    if(m_VideoOutput && m_VideoDecoder && m_View)
    {
        int m_Width = m_VideoDecoder->GetWidth();
        int m_Height = m_VideoDecoder->GetHeight();
        m_View->Config(m_Width, m_Height);
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

bool player::ConfigAudioOutput()
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

int player::decode_packet(UniquePacketPtr pkt, const bool IsFlushDecoder)
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

double player::GetAudioClock()
{
    double audioclock{};
    if(m_View != nullptr)
    {
        audioclock = m_View->get_clock();
    }
    return audioclock;
}

AVRational player::GetTimeBaseAudio()
{
    if(m_Demuxer == nullptr)
    {
        LOGE("Demuxer is nullptr");
        return {};
    }
    return m_Demuxer->GetTimeBaseAudio();
}

AVRational player::GetTimeBaseVideo()
{
    if(m_Demuxer == nullptr)
    {
        LOGE("Demuxer is nullptr");
        return {};
    }
    return m_Demuxer->GetTimeBaseVideo();
}

std::atomic<PlayerState>& player::GetCurrentState()
{
    return m_PlayerState;
}

bool player::InitVideoDecoder(const AVCodecID codecID, AVCodecParameters* codec_par)
{
    if(m_VideoDecoder && (m_VideoDecoder->init_decoder(codecID, codec_par) != 0))
    {
        LOGE("Init Video Decoder fail");
        return false;
    }
    return true;
}

bool player::InitAudioDecoder(const AVCodecID codecID, AVCodecParameters* codec_par)
{
    if(m_AudioDecoder && (m_AudioDecoder->init_decoder(codecID, codec_par) != 0))
    {
        LOGE("Init Audio Decoder fail");
        return false;
    }
    return true;
}

void player::PushSDLAudioData(const uint8_t* data, size_t size)
{
    if(m_View == nullptr)
    {
        LOGE("view ptr is null");
    }
    m_View->push(data, size);
}

bool player::AudioConfig(int sample_rate,
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
    return m_View->config(sample_rate, channels, format, first_pts, samples);
}