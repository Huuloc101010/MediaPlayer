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
    exit(0);
}

void player::EventStop()
{
    switch(m_PlayerState.load())
    {

        case PlayerState::PLAYING:
        case PlayerState::PAUSED:
        {
            if(m_Demuxer)      m_Demuxer     ->controlfunction::Stop();
            if(m_VideoOutput)  m_VideoOutput ->controlfunction::Stop();
            if(m_AudioOutput)  m_AudioOutput ->controlfunction::Stop();
            if(m_VideoDecoder) m_VideoDecoder->controlfunction::Stop();
            if(m_AudioDecoder) m_AudioDecoder->controlfunction::Stop();
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
        if(m_Demuxer)      m_Demuxer      ->Exit();
        if(m_VideoOutput)  m_VideoOutput  ->Exit();
        if(m_AudioOutput)  m_AudioOutput  ->Exit();
        if(m_VideoDecoder) m_VideoDecoder ->Exit();
        if(m_AudioDecoder) m_AudioDecoder ->Exit();
        if(m_Controller)   m_Controller   ->Exit();
        break;
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

const std::string player::err2str(int errnum)
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
    m_Demuxer      = std::make_unique<demuxer>(this);
    m_VideoDecoder = std::make_unique<videodecoder>(this);
    m_AudioDecoder = std::make_unique<audiodecoder>(this);
    m_VideoOutput  = std::make_unique<videooutput>(this);
    m_AudioOutput  = std::make_unique<audiooutput>(this);
    m_Controller   = std::make_unique<controller>(this);
    m_View         = std::make_unique<view>();
    int Ret = -1;
    if(m_Demuxer != nullptr)
    {
        Ret = m_Demuxer->StartPlay(m_CurrentMedia);
    }
    
    LOGI("Demuxing succeeded");
    return 0;
}

bool player::ConfigVideoOutput()
{
    /* allocate image where the decoded image will be put */
    if(m_VideoOutput)
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
    if(m_AudioOutput != nullptr)
    {
        audioclock = m_AudioOutput->get_clock();
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
